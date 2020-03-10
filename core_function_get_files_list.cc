#include <stdexcept>

#include "core_function_get_files_list.h"

#include "xml_string_parameter.h"
#include "xml_result_ok.h"
#include "xml_result_bad_request.h"
#include "xml_result_not_found.h"
#include "xml_query_get_files_list.h"
#include "query_client.h"
#include "file_handler.h"
#include "utils.h"

CoreFunctionGetFilesList::CoreFunctionGetFilesList(
    XmlParameterList parameters,
    Glib::RefPtr <XmlDescription> description,
    const Glib::ustring & textbody)
    : CoreFunctionCall ("getFilesList", parameters, description, textbody)
{ }

CoreFunctionGetFilesList::~CoreFunctionGetFilesList ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionGetFilesList::factory(XmlParameterList parameters,
                                  Glib::RefPtr <XmlDescription> description,
				  const Glib::ustring & textbody,
				  const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr<CoreFunctionCall> (new CoreFunctionGetFilesList
                                           (parameters,
                                            description,
					    textbody));
}

std::vector<Glib::ustring>
CoreFunctionGetFilesList::build_reply_xml(const std::string& dir, const std::string& host,
                                          const std::vector<std::string>& dirs) const
{
    std::vector<Glib::ustring> result;

    for (auto&& val : dirs) {
        Glib::ustring prefixed_dir, xml;

        if (dir[dir.size() - 1] == '/')
            prefixed_dir = Glib::ustring::compose("%1%2", dir, val);
        else
            prefixed_dir = Glib::ustring::compose("%1/%2", dir, val);

        if (host == "DC")
            xml = Glib::ustring::compose("<file id=\"%1\" />", prefixed_dir);
        else
            xml = Glib::ustring::compose("<file host=\"SIC\" id=\"%1\" />", prefixed_dir);

        result.push_back(xml);
    }

    return result;
}

void CoreFunctionGetFilesList::on_write_query_finish(
    const Glib::RefPtr<XmlResult>& result)
{
    // failure?
    if (result->get_status() != 200)
        PRINT_ERROR("GetFilesList on DC failed");

    // just forward result
    call_finished(result);
}

void CoreFunctionGetFilesList::start_call()
{
    // host and dir are optional
    Glib::ustring host, dir;
    auto host_param = _parameters.get<XmlStringParameter>("host");
    auto dir_param  = _parameters.get<XmlStringParameter>("dir");

    host = host_param ? host_param->get_str() : "DC";
    dir  = dir_param  ? dir_param->get_str()  : "";

    // consistency checks
    if (host != "DC" && host != "SIC") {
        XML_RESULT_BAD_REQUEST("Host has invalid value");
        return;
    }

    if (host == "DC" && dir != "") {
        XML_RESULT_BAD_REQUEST("Dir is invalid for DC");
        return;
    }

    if (host == "SIC" && dir == "") {
        XML_RESULT_BAD_REQUEST("No Dir given for SIC");
        return;
    }

    // do directory listing
    try {
        if (host == "SIC") {
            auto files  = FileHandler::list_directory_files(dir);
            auto result_xml = build_reply_xml(dir, host, files);

            XML_RESULT_OK_RET("", result_xml);
        } else {
            auto dc = QueryClient::get_instance();
            auto xq = XmlQueryGetFilesList::create();
            _write_query = dc->create_query(xq);
            _write_query->finished.connect(
                sigc::mem_fun(*this, &CoreFunctionGetFilesList::on_write_query_finish));
            dc->execute(_write_query);
        }
    } catch (const std::exception& ex) {
        XML_RESULT_NOT_FOUND(ex.what());
    }
}
