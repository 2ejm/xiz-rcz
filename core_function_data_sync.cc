#include <glibmm/regex.h>

#include <stdexcept>
#include <iostream>
#include <vector>
#include <regex>

#include "xml_string_parameter.h"
#include "xml_file.h"
#include "xml_measurement.h"
#include "xml_result_ok.h"
#include "xml_result_bad_request.h"
#include "xml_result_not_found.h"
#include "xml_result_forbidden.h"
#include "xml_result_internal_device_error.h"
#include "file_handler.h"
#include "conf_handler.h"
#include "id_mapper.h"
#include "utils.h"

#include "core_function_data_sync.h"

CoreFunctionDataSync::CoreFunctionDataSync(
    XmlParameterList parameters,
    Glib::RefPtr <XmlDescription> description,
    const Glib::ustring& text)
    : CoreFunctionCall ("dataSync", parameters, description, text)
{ }

CoreFunctionDataSync::~CoreFunctionDataSync ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionDataSync::factory(XmlParameterList parameters,
                              Glib::RefPtr <XmlDescription> description,
                              const Glib::ustring & text,
			      const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr<CoreFunctionCall>(
        new CoreFunctionDataSync(parameters, description, text));
}

bool CoreFunctionDataSync::check_xml_and_get_values()
{
    const auto& iid_param  = _parameters.get<XmlStringParameter>("iid");
    const auto& type_param = _parameters.get<XmlStringParameter>("type");
    const auto& tag_param  = _parameters.get<XmlStringParameter>("tag");
    const auto& dap_param  = _parameters.get<XmlStringParameter>("dap");
    const auto& car_param  = _parameters.get<XmlStringParameter>("car");
    const auto& id_param   = _parameters.get<XmlStringParameter>("id");
    const auto& date_param = _parameters.get<XmlStringParameter>("stamp_date");
    const auto& time_param = _parameters.get<XmlStringParameter>("stamp_time");
    const auto& tz_param   = _parameters.get<XmlStringParameter>("stamp_timeZone");

    _iid  = iid_param  ? iid_param->get_str()  : "";
    _type = type_param ? type_param->get_str() : "";
    _tag  = tag_param  ? tag_param->get_str()  : "";
    _dap  = dap_param  ? dap_param->get_str()  : "";
    _car  = car_param  ? car_param->get_str()  : "";
    _id   = id_param   ? id_param->get_str()   : "";
    _date = date_param ? date_param->get_str() : "";
    _time = time_param ? time_param->get_str() : "";
    _tz   = tz_param   ? tz_param->get_str()   : "";

    if (_iid.empty())
        return false;

    if (_type.empty())
        _type = "NUM";

    if (_type != "NUM" && _type != "IMG")
        return false;

    return true;
}

std::string CoreFunctionDataSync::build_file_name() const
{
    std::string extension, file_name;

    if (_type == "NUM")
        extension = ".xml";
    else
        extension = ".bmp";

    file_name  = _iid;
    file_name += _tag;

    return file_name + extension;
}

std::string CoreFunctionDataSync::build_pp_file_name(const Format& format) const
{
    auto conf_handler = ConfHandler::get_instance();
    std::string dir   = conf_handler->getDirectory("measurements");
    std::string path;

    if (dir.empty())
        EXCEPTION("Couldn't find measurement directory");

    // build path
    path  = dir + "/" + _iid + "/";
    path += _iid;
    path += format.tag();
    path += ".";
    path += format.ext();

    return path;
}

std::string CoreFunctionDataSync::setup_measurement_dir() const
{
    auto        conf_handler = ConfHandler::get_instance();
    std::string dir          = conf_handler->getDirectory("measurements");

    if (dir.empty())
        EXCEPTION("Couldn't find measurement directory");

    dir += "/";
    dir += _iid;

    if (!FileHandler::directory_exists(dir))
        FileHandler::create_directory(dir);

    return dir;
}

CoreFunctionDataSync::StringPair CoreFunctionDataSync::split_car_attribute() const
{
    std::vector<std::string> tokens = Glib::Regex::split_simple(":", _car);

    if (tokens.size() != 2)
        EXCEPTION("Failed to parse create and return attribute: " << _car);

    return { tokens[0], tokens[1] };
}

CoreFunctionDataSync::StringPair CoreFunctionDataSync::get_and_strip_id(const std::string& stdout) const
{
    std::regex re("id=\"(.*?)\"");
    std::smatch match;
    std::string id;

    if (std::regex_search(stdout, match, re)) {
        std::regex re_replace("id=\".*\"");
        std::string new_stdout;

        id = match[1];
        new_stdout = std::regex_replace(
            stdout, re_replace, "", std::regex_constants::format_first_only);
        return { id, new_stdout };
    }

    return { "", stdout };
}

PostProcessor CoreFunctionDataSync::get_postprocessor(const std::string& format) const
{
    auto conf_handler = ConfHandler::get_instance();

    return conf_handler->getSpecificPostProcessor(format);
}

std::string CoreFunctionDataSync::get_database_script() const
{
    auto conf_handler = ConfHandler::get_instance();
    auto db_script    = conf_handler->getAllocationProcessor(_dap);

    return db_script;
}

void CoreFunctionDataSync::add_id_mapping(const Glib::ustring& id) const
{
    try {
        auto id_mapper = IdMapper::get_instance();
        id_mapper->add_mapping({ id.empty() ? _id : id, _iid, _date, _time, _tz });
    } catch (const std::exception& ex) {
        PRINT_ERROR("Inserting Id Mapping failed: " << ex.what());
    }
}

void CoreFunctionDataSync::handle_xml(const std::string& file_name)
{
    auto measurement_param = _parameters.get<XmlMeasurement>("measurement");

    if (!measurement_param) {
        XML_RESULT_BAD_REQUEST("No measurement data for type NUM found");
        return;
    }

    _write_req = WriteFileRequest::create(
        file_name, measurement_param->get_content());
    _write_req->finished.connect(
        sigc::mem_fun(*this, &CoreFunctionDataSync::on_write_finish));
    _write_req->start_write();
}

void CoreFunctionDataSync::handle_img(const std::string& file_name)
{
    // base64 decode
    std::string decoded = FileHandler::base64_decode(_textbody);

    // write file, but asyncly
    _write_req = WriteFileRequest::create(file_name, decoded);
    _write_req->finished.connect(
        sigc::mem_fun(*this, &CoreFunctionDataSync::on_write_finish));
    _write_req->start_write();
}

void CoreFunctionDataSync::delete_post_processing_results() const
{
    auto        conf_handler = ConfHandler::get_instance();
    std::string dir          = conf_handler->getDirectory("measurements");

    if (dir.empty()) {
        PRINT_ERROR("Couldn't find measurement directory");
        return;
    }

    dir += "/";
    dir += _iid;

    try {
        auto files = FileHandler::list_directory(dir);

        for (auto&& file : files) {
            const std::string xml_pattern = _iid + "\\.xml";
            const std::string bmp_pattern = _iid + ".*?\\.bmp";

            std::regex re_xml(xml_pattern), re_bmp(bmp_pattern);
            std::smatch match;

            if (!std::regex_match(file, match, re_xml) &&
                !std::regex_match(file, match, re_bmp))
                FileHandler::del_file(dir + "/" + file);
        }
    } catch (const std::exception& ex) {
        PRINT_ERROR(ex.what());
    }
}

void CoreFunctionDataSync::on_write_finish(const Glib::RefPtr<WriteFileResult>& result)
{
    // write successful?
    if (result->error()) {
        XML_RESULT_FORBIDDEN(result->error_msg());
        return;
    }

    // write complete -> cleanup pp'ed results
    delete_post_processing_results();

    // write complete -> add mapping
    add_id_mapping();

    // file write complete! Done?
    if (_dap.empty() && _car.empty()) {
        XML_RESULT_OK("");
        return;
    }

    // db insert?
    if (!_dap.empty()) {
        start_db_proc();
        return;
    }

    // car?
    if (!_car.empty())
        start_car_proc();
}

void CoreFunctionDataSync::start_db_proc()
{
    try {
        auto db_script = get_database_script();
        if (db_script.empty()) {
            XML_RESULT_BAD_REQUEST("Couldn't find referenced Database Script");
            return;
        }

        _proc_req = ProcessRequest::create(
            { db_script, "--iid",  _iid }, ProcessRequest::DEFAULT_TIMEOUT, true);
        _proc_req->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionDataSync::on_database_finish));
        _proc_req->start_process();
    } catch (const std::exception& ex) {
        XML_RESULT_INTERNAL_DEVICE_ERROR(
            "Failed to start Database linking script: " << ex.what());
    }
}

void CoreFunctionDataSync::start_car_proc()
{
    try {
        auto pair = split_car_attribute();
        auto pp   = get_postprocessor(pair.second);

        if (pp.code().empty()) {
            XML_RESULT_BAD_REQUEST("Couldn't find referenced Postprocessor Script");
            return;
        }

        _generated_file = build_pp_file_name(pp.get_format_by_name(pair.second));

        _proc_req = ProcessRequest::create(
            { pp.code(), "--iid",  _iid, "--format", pair.second },
            ProcessRequest::DEFAULT_TIMEOUT, true, _generated_file);
        _proc_req->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionDataSync::on_car_finish));
        _proc_req->start_process();
    } catch (const std::exception& ex) {
        XML_RESULT_INTERNAL_DEVICE_ERROR(
            "Failed to start Postprocessor script: " << ex.what());
    }
}

void CoreFunctionDataSync::on_database_finish(const Glib::RefPtr<ProcessResult>& result)
{
    std::stringstream ss;
    std::string msg;

    auto id = get_and_strip_id(result->stdout());

    ss << "<dap exit=\"" << result->get_child_status() << "\" id=\"" << id.first << "\">\n";
    ss << "<![CDATA[\n"  << id.second << "]]>\n";
    ss << "</dap>";
    msg = ss.str();

    _ret_value.emplace_back(msg);

    // update mapping if id found by dap
    if (!id.first.empty())
        add_id_mapping(id.first);

    // create and return?
    if (!_car.empty())
        start_car_proc();
    else
        XML_RESULT_OK_RET("", _ret_value);
}

void CoreFunctionDataSync::on_car_finish(const Glib::RefPtr<ProcessResult>& result)
{
    std::stringstream ss;
    std::string msg;

    ss << "<car exit=\"" << result->get_child_status() << "\">\n";
    ss << "<![CDATA[\n"  << result->stdout() << "]]>\n";
    ss << "</car>";

    msg = ss.str();

    _ret_value.emplace_back(msg);

    // done!
    XML_RESULT_OK_RET("", _ret_value);
}

void CoreFunctionDataSync::start_call()
{
    std::string dir;

    if (!check_xml_and_get_values()) {
        XML_RESULT_BAD_REQUEST("Malformed XML received");
        return;
    }

    // get filename
    auto file_name = build_file_name();

    // setup measurement directory
    try {
        dir = setup_measurement_dir();
    } catch (const std::exception& ex) {
        XML_RESULT_INTERNAL_DEVICE_ERROR(ex.what());
        return;
    }
    file_name = dir + "/" + file_name;

    // create it
    if (_type == "IMG")
        handle_img(file_name);
    else
        handle_xml(file_name);
}
