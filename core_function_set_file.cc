#include <stdexcept>
#include <iostream>

#include "core_function_set_file.h"

#include "query_client_serial.h"
#include "xml_string_parameter.h"
#include "xml_file.h"
#include "xml_result_ok.h"
#include "xml_result_bad_request.h"
#include "xml_result_not_found.h"
#include "xml_result_forbidden.h"
#include "xml_query_set_file.h"
#include "query_client.h"
#include "file_handler.h"
#include "utils.h"

CoreFunctionSetFile::CoreFunctionSetFile(
    XmlParameterList parameters,
    Glib::RefPtr <XmlDescription> description,
    const Glib::ustring & textbody)
    : CoreFunctionCall ("setFile", parameters, description, textbody)
{ }

CoreFunctionSetFile::~CoreFunctionSetFile ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionSetFile::factory(XmlParameterList parameters,
                             Glib::RefPtr <XmlDescription> description,
                             const Glib::ustring & textbody,
			     const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr<CoreFunctionCall> (
        new CoreFunctionSetFile (parameters, description, textbody));
}

void CoreFunctionSetFile::check_file_list() const
{
    for (auto&& file : _files) {
        const auto& id      = file->get_id();
        const auto& content = file->get_content();
        const auto& host    = file->get_host();

        if (!host.empty() && host != "DC" && host != "SIC")
            EXCEPTION("Host " << host << " is unknown");

        if (id.empty())
            EXCEPTION("No <id> for file given");

        if (content.empty())
            EXCEPTION("No content for file given");
    }

    if (!_files.size())
        EXCEPTION("No file tags given");
}

void CoreFunctionSetFile::handle_one_file(Glib::RefPtr<XmlFile> file)
{
    /*
     * Note: Host can also be given globally which overrides the file's host.
     */
    const auto& host_param = _parameters.get<XmlStringParameter>("host");
    const auto& global_host = host_param ? host_param->get_str() : "";
    const auto& host = global_host.empty() ? file->get_host() : global_host;
    const auto& update_param = _parameters.get<XmlStringParameter>("update");

    if (host == "SIC") {
        const auto decoded = FileHandler::base64_decode(file->get_content());
        _write_proc = WriteFileRequest::create(file->get_id(), decoded);
        _write_proc->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionSetFile::on_write_finish));
        _write_proc->start_write();
    } else if (host == "DC" || host.empty()) {
        auto dc = QueryClient::get_instance();
	Glib::RefPtr <XmlQuery> xq;

	if (update_param)
	    xq = XmlQuerySetFile::create (file, update_param->get_str());
	else
	    xq = XmlQuerySetFile::create(file);

        _write_query = dc->create_query(xq);
        _write_query->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionSetFile::on_write_query_finish));
        dc->execute(_write_query);
    }
}

void CoreFunctionSetFile::on_write_query_finish(
    const Glib::RefPtr<XmlResult>& result)
{
    // failure?
    if (result->get_status() != 200) {
        PRINT_ERROR("SetFile on DC failed");
        call_finished(result);
        return;
    }

    // start next write
    if (++_proc_write_it != _files.cend()) {
        handle_one_file(*_proc_write_it);
        return;
    }

    XML_RESULT_OK("");
}

void CoreFunctionSetFile::on_write_finish(
    const Glib::RefPtr<WriteFileResult>& result)
{
    // failure?
    if (result->error()) {
        XML_RESULT_FORBIDDEN(result->error_msg());
        return;
    }

    // start next write
    if (++_proc_write_it != _files.cend()) {
        handle_one_file(*_proc_write_it);
        return;
    }

    XML_RESULT_OK("");
}

void CoreFunctionSetFile::start_call ()
{
    try {
        _files = _parameters.get_all<XmlFile>("file");

        check_file_list();

        _proc_write_it = _files.cbegin();
        handle_one_file(*_proc_write_it);
    } catch (const std::exception& ex) {
        XML_RESULT_BAD_REQUEST(ex.what());
    }
}
