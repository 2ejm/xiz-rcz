#include <stdexcept>
#include <iostream>

#include <glibmm/fileutils.h>
#include <giomm/file.h>

#include "core_function_get_file.h"

#include "xml_string_parameter.h"
#include "xml_file.h"
#include "xml_result_ok.h"
#include "xml_result_bad_request.h"
#include "xml_result_not_found.h"
#include "xml_query_get_file.h"
#include "query_client.h"
#include "file_handler.h"
#include "utils.h"

CoreFunctionGetFile::CoreFunctionGetFile(
    XmlParameterList parameters,
    Glib::RefPtr <XmlDescription> description,
     const Glib::ustring & textbody)
    : CoreFunctionCall ("getFile", parameters, description, textbody)
{ }

CoreFunctionGetFile::~CoreFunctionGetFile ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionGetFile::factory(XmlParameterList parameters,
                             Glib::RefPtr <XmlDescription> description,
                             const Glib::ustring & textbody,
			     const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr<CoreFunctionCall> (new CoreFunctionGetFile
                                           (parameters,
                                            description,
					    textbody));
}

void CoreFunctionGetFile::addDirectory( Glib::ustring directoryName)
{
    Glib::Dir dir (directoryName);

    //std::list<std::string> entries (dir.begin(), dir.end());

    for(auto&& dirEntry: dir)
    {
        Glib::ustring fileName=Glib::ustring::compose("%1/%2", directoryName, dirEntry );
        PRINT_DEBUG( "DIORECTORY " << directoryName << dirEntry);
        Glib::RefPtr<Gio::File> file=Gio::File::create_for_path( fileName );

        try
        {
            if( file->query_file_type() == Gio::FILE_TYPE_DIRECTORY )
            {
                addDirectory(fileName);
            }
            else
                _files.push_back( XmlFile::create( fileName, "SIC") );
        }
        catch ( ... )
        {
            PRINT_ERROR("Could not add directory");
        }
    }

    PRINT_DEBUG( "---" );
}

void CoreFunctionGetFile::check_file_list() const
{
    for (auto&& file : _files) {
        const auto& id      = file->get_id();
        const auto& host    = file->get_host();

        if (!host.empty() && host != "DC" && host != "SIC")
            EXCEPTION("Host " << host << " is unknown");

        if (id.empty())
            EXCEPTION("No <id> for file given");
    }
}

void CoreFunctionGetFile::handle_one_file(const Glib::RefPtr<XmlFile>& file)
{
    /*
     * Note: Host can also be given globally which overrides the file's host.
     */
    const auto& host_param = _parameters.get<XmlStringParameter>("host");
    const auto& global_host = host_param ? host_param->get_str() : "";
    const auto& host = global_host.empty() ? file->get_host() : global_host;

    if (host == "" || host == "DC") {
        auto dc = QueryClient::get_instance();
        auto xq = XmlQueryGetFile::create(file->get_id());
        _write_query = dc->create_query(xq);
        _write_query->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionGetFile::on_write_query_finish));
        dc->execute(_write_query);
    } else {
        _read_proc = ReadFileRequest::create(file->get_id());
        _read_proc->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionGetFile::on_read_finish));
        _read_proc->start_read();
    }
}

void CoreFunctionGetFile::on_write_query_finish(
    const Glib::RefPtr<XmlResult>& result)
{
    // failure?
    if (result->get_status() != 200) {
        PRINT_ERROR("GetFile on DC failed");
        call_finished(result);
        return;
    }

    // save data
    _return_values.push_back(result->to_body());

    // start next read
    if (++_proc_read_it != _files.cend()) {
        handle_one_file(*_proc_read_it);
        return;
    }

    XML_RESULT_OK_RET("", _return_values);
}

void CoreFunctionGetFile::on_read_finish(const Glib::RefPtr<ReadFileResult>& result)
{
    // failure?
    if (result->error()) {
        XML_RESULT_NOT_FOUND(result->error_msg());
        return;
    }

    // save data
    auto encoded = FileHandler::base64_encode(result->content());
    const auto& file = *_proc_read_it;

    if (file->get_host().empty())
        _return_values.push_back(
            Glib::ustring::compose("<file id=\"%1\">%2</file>",
                                   file->get_id(), encoded));
    else
        _return_values.push_back(
            Glib::ustring::compose("<file host=\"%1\" id=\"%2\">%3</file>",
                                   file->get_host(), file->get_id(), encoded));

    // continue with next file
    if (++_proc_read_it != _files.cend()) {
        handle_one_file(*_proc_read_it);
        return;
    }

    XML_RESULT_OK_RET("", _return_values);
}

void CoreFunctionGetFile::start_call()
{
    try {
        auto host_param = _parameters.get<XmlStringParameter>("host");
        auto id_param   = _parameters.get<XmlStringParameter>("id");

        _files = _parameters.get_all<XmlFile>("file");

        // add file given by id
        if (id_param)
            _files.push_front(
                XmlFile::create(
                    id_param->get_str(), host_param ? host_param->get_str() : ""));

        check_file_list();

        // Exporting roofs for SIC does not make sense, way to big
        if (!_files.size() && ( host_param->get_str() == "SIC" ) )
            EXCEPTION("Cant export whole root file system. Too large.");

        _proc_read_it = _files.cbegin();

        if(_files.size())
            handle_one_file(*_proc_read_it);
        else
            // Special call "all"
            handle_one_file( XmlFile::create( Glib::ustring(), host_param->get_str() ) );
    } catch (const std::exception& ex) {
        XML_RESULT_BAD_REQUEST(ex.what());
    }
}
