#include <stdexcept>
#include <iostream>

#include "core_function_del_file.h"
#include "xml_string_parameter.h"
#include "xml_result_ok.h"
#include "xml_result_bad_request.h"
#include "xml_result_not_found.h"
#include "xml_query_del_file.h"
#include "query_client.h"
#include "file_handler.h"
#include "utils.h"

CoreFunctionDelFile::CoreFunctionDelFile(
    XmlParameterList parameters,
    Glib::RefPtr <XmlDescription> description,
     const Glib::ustring & textbody)
    : CoreFunctionCall ("delFile", parameters, description, textbody)
{ }

CoreFunctionDelFile::~CoreFunctionDelFile ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionDelFile::factory(XmlParameterList parameters,
                                  Glib::RefPtr <XmlDescription> description,
				  const Glib::ustring & textbody,
				  const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr<CoreFunctionCall> (new CoreFunctionDelFile
                                           (parameters,
                                            description,
					    textbody));
}

void CoreFunctionDelFile::check_file_list() const
{
    for (auto&& file : _files) {
        const auto& id   = file->get_id();
        const auto& host = file->get_host();

        if (!host.empty() && host != "DC" && host != "SIC")
            EXCEPTION("Host " << host << " is unknown");

        if (id.empty())
            EXCEPTION("No <id> for file given");
    }

    if (!_files.size())
        EXCEPTION("No files given");
}

void CoreFunctionDelFile::handle_one_file(const Glib::RefPtr<XmlFile>& file)
{
    /*
     * Note: Host can also be given globally which overrides the file's host.
     */
    const auto& host_param  = _parameters.get<XmlStringParameter>("host");
    const auto& global_host = host_param ? host_param->get_str() : "";
    const auto& host        = global_host.empty() ? file->get_host() : global_host;

    if (host == "" || host == "DC") {
        auto dc = QueryClient::get_instance();
        auto xq = XmlQueryDelFile::create(file->get_id());
        _write_query = dc->create_query(xq);
        _write_query->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionDelFile::on_write_query_finish));
        dc->execute(_write_query);
    } else {
        try {
            FileHandler::del_file(file->get_id());
        } catch (const std::exception& ex) {
            XML_RESULT_NOT_FOUND(ex.what());
            return;
        }

        ++_del_it;

        if (_del_it != _files.cend()) {
            handle_one_file(*_del_it);
        } else {
            XML_RESULT_OK("");
        }
    }
}

void CoreFunctionDelFile::on_write_query_finish(
    const Glib::RefPtr<XmlResult>& result)
{
    // failure?
    if (result->get_status() != 200) {
        PRINT_ERROR("DelFile on DC failed");
        call_finished(result);
        return;
    }

    // start next write
    if (++_del_it != _files.cend()) {
        handle_one_file(*_del_it);
        return;
    }

    XML_RESULT_OK("");
}

void CoreFunctionDelFile::start_call ()
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

        _del_it = _files.cbegin();

        handle_one_file(*_del_it);
    } catch (const std::exception& ex) {
        XML_RESULT_BAD_REQUEST(ex.what());
    }
}
