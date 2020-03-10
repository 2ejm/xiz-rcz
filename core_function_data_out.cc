#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>

#include "xml_string_parameter.h"
#include "xml_file.h"
#include "xml_measurement.h"
#include "xml_result_ok.h"
#include "xml_result_bad_request.h"
#include "xml_result_not_found.h"
#include "xml_result_forbidden.h"
#include "xml_result_internal_device_error.h"
#include "xml_helpers.h"
#include "time_utilities.h"
#include "file_handler.h"
#include "log_handler.h"
#include "conf_handler.h"
#include "id_mapper.h"
#include "utils.h"

#include "core_function_data_out.h"

CoreFunctionDataOut::CoreFunctionDataOut(
    XmlParameterList parameters,
    Glib::RefPtr <XmlDescription> description,
    const Glib::ustring & text,
    const xmlpp::Element *en)
    : CoreFunctionCall ("dataOut", parameters, description, text)
    , _en(en)
{ }

CoreFunctionDataOut::~CoreFunctionDataOut ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionDataOut::factory(XmlParameterList parameters,
                             Glib::RefPtr <XmlDescription> description,
                             const Glib::ustring & text,
                             const xmlpp::Element *en)
{
    return Glib::RefPtr<CoreFunctionCall>(
        new CoreFunctionDataOut(parameters, description, text, en));
}

void CoreFunctionDataOut::process_xml_params()
{
    const auto& type_param     = _parameters.get<XmlStringParameter>("type");
    const auto& id_param       = _parameters.get<XmlStringParameter>("id");
    const auto& iid_param      = _parameters.get<XmlStringParameter>("iid");
    const auto& format_param   = _parameters.get<XmlStringParameter>("format");
    const auto& dest_param     = _parameters.get<XmlStringParameter>("dest");
    const auto& output_param   = _parameters.get<XmlStringParameter>("output");
    const auto& protocol_param = _parameters.get<XmlStringParameter>("protocol");
    const auto& scheme_param   = _parameters.get<XmlStringParameter>("scheme");

    _type     = type_param     ? type_param->get_str()     : "measurement";
    _id       = id_param       ? id_param->get_str()       : "";
    _iid      = iid_param      ? iid_param->get_str()      : "";
    _format   = format_param   ? format_param->get_str()   : "";
    _output   = output_param   ? output_param->get_str()   : "";
    _protocol = protocol_param ? protocol_param->get_str() : "";
    _scheme   = scheme_param   ? scheme_param->get_str()   : "";

    _dest = FileDestination(dest_param ? dest_param->get_str() : "");
}

bool CoreFunctionDataOut::check_valid_xml() const
{
    // at least, dest should be given and known
    if (_dest == FileDestination::Dest::unknown)
        return false;

    // type
    if (_type != "measurement" && _type != "log" &&
        _type != "monitor" && _type != "conf")
        return false;

    // measurement? we need at least one id
    if (_type == "measurement" && _id.empty() && _iid.empty())
        return false;

    return true;
}

std::vector<std::string> CoreFunctionDataOut::get_copy_script() const
{
    std::vector<std::string> result;
    auto conf_handler = ConfHandler::get_instance();

    if ( _scheme.empty() && (conf_handler->findElement(".//interface/resource[@id='"
                            + _dest.to_string()
                            + "']/protocol[@id='"+_protocol+"']/scheme") ) )
        EXCEPTION("For this copy protocol scheme has to be given.");

    // two cases: lanSocket and serial
    if (_dest == FileDestination::Dest::serial) {
        auto handler = conf_handler->getSerialProtocolHandler(_protocol);
        if (handler.code().empty())
            EXCEPTION("Couldn't find serial copy script in configuration.");

        result.push_back(handler.code());
        result.push_back("--protocol");
        result.push_back(handler.id());
    }
    if (_dest == FileDestination::Dest::socket) {
        auto handler = conf_handler->getLanProtocolHandler(_protocol);
        if (handler.code().empty())
            EXCEPTION("Couldn't find lan copy script in configuration.");

        result.push_back(handler.code());
        result.push_back("--protocol");
        result.push_back(handler.id());
    }

    // common parameters for lan and serial
    if (!_scheme.empty()) {
        result.push_back("--scheme");
        result.push_back(_scheme);
    }

    result.push_back("--type");
    result.push_back(_type);
    result.push_back("--iid");
    result.push_back(_iid);
    result.push_back("--format");
    result.push_back(_format);

    return result;
}

std::string CoreFunctionDataOut::get_serial_number()
{
    if (!_serial_number.empty())
        return _serial_number;

    auto conf_handler = ConfHandler::get_instance();
    auto ret = conf_handler->getParameter("serialnumber");

    if (ret.empty())
        EXCEPTION("Failed to lookup serial number in configuration");

    _serial_number = ret;

    return ret;
}

void CoreFunctionDataOut::create_post_processing_queue()
{
    // map id to iid if needed
    if (_iid.empty()) {
        auto id_mapper = IdMapper::get_instance();
        _iid = id_mapper->get_iid(_id);
    }
    _pp_queue = PostProcessingBuilder().build_queue(_iid, _dest, _format, _output);
}

void CoreFunctionDataOut::create_copy_queue()
{
    for (auto&& pp_entry : _pp_queue)
        _copy_queue.emplace_back(
            pp_entry.generated_file, pp_entry.output_file);
}

bool CoreFunctionDataOut::is_post_processing_needed(const std::string& file) const
{
    return !FileHandler::file_exists(file);
}

std::string CoreFunctionDataOut::create_config_xml()
{
    if (!_dest.is_file_based_dest())
        EXCEPTION("Config cannot be copied to serial or printer");

    auto conf_handler = ConfHandler::get_instance();

    auto folder = conf_handler->getFolder(_dest, "conf");
    if (folder.size() != 1)
        EXCEPTION("Failed to get config folder via zixconf.xml");

    lHighDebug ("creating Config file\n");
    std::string root_folder = conf_handler->getRootFolder(_dest);
    std::string conf_folder = folder[0].path();
    std::string file_name = get_serial_number() + "__" +
        TimeUtilities::get_timestamp_log_format() + "_conf.xml";
    std::string dir  = root_folder + "/" + conf_folder;
    std::string path = dir + "/" + file_name;

    // make sure target directory exists
    if (!FileHandler::directory_exists(dir))
        FileHandler::create_directory(dir);

    // DC part of config is stored in body which is stored in _parameters
    std::string dc_content;
    std::stringstream ss;
    ss << "<dc>";
    for (auto c : _en->get_children ()) {
        const xmlpp::Element *element = dynamic_cast <xmlpp::Element *> (c);
        const xmlpp::TextNode *text = dynamic_cast <xmlpp::TextNode *> (c);
        if (element) {
            ss << element_to_string(element, 0, 1);
        } else {
            if (!text)
                EXCEPTION("Failed to interpret XML from DC");
            ss << text->get_content();
        }
    }
    ss << "</dc>\n";
    dc_content = ss.str();

    // SIC part is zixconf.xml
    auto sic_content = FileHandler::get_file(conf_handler->get_config_file_path());

    xmlpp::DomParser dc_parser, sic_parser;
    dc_parser.parse_memory(dc_content);
    sic_parser.parse_memory(sic_content);
    xmlpp::Document doc;
    auto *root = doc.create_root_node("conf");

    // add signature
    auto *sig_node = root->add_child("signature");
    sig_node->add_child_text("foo");

    // add dc section
    root->import_node(dc_parser.get_document()->get_root_node(), true);

    // add sic section
    root->import_node(sic_parser.get_document()->get_root_node(), true);

    doc.write_to_file_formatted(path);

    return path;
}

void CoreFunctionDataOut::start_postprocessing()
{
    std::string generated_file;

    // start it
    try {
        _post_proc_idx = 0;
        const auto& entry = _pp_queue.at(_post_proc_idx);

	/* save generated_file, becesue we might need to delete
	 * it in case of error
	 */
	generated_file = entry.generated_file;

        _post_proc = ProcessRequest::create(
            { entry.post_processor, "--iid", _iid, "--format", entry.format },
            ProcessRequest::DEFAULT_TIMEOUT, false, entry.generated_file);
        _post_proc->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionDataOut::on_post_proc_finish));
        if (!is_post_processing_needed(entry.generated_file))
            _post_proc->finished.emit(ProcessResult::create(0, "", "", true, 0));
        else
            _post_proc->start_process();
    } catch (const std::exception& ex) {
        auto xml_res = XmlResultInternalDeviceError::create(
            Glib::ustring::compose(
                "Couldn't start postprocessor script: %1", ex.what()));
	if (!generated_file.empty ())
	    FileHandler::del_file (generated_file);
        finished.emit(xml_res);
    }
}

void CoreFunctionDataOut::start_file_copying()
{
    _copy_req_idx = 0;
    const auto& entry = _copy_queue[_copy_req_idx];

    _copy_req = CopyFileRequest::create(entry.src, entry.dest);
    _copy_req->finished.connect(
        sigc::mem_fun(*this, &CoreFunctionDataOut::on_copy_finish));
    _copy_req->start_copy();
}

void CoreFunctionDataOut::start_printer_copying()
{
    std::vector<std::string> lp_args{ "lp", "-o", "fit-to-page" };

    // try to find configured printer
    auto conf_handler = ConfHandler::get_instance();
    auto printer_name = conf_handler->getParameter("lanPrinterName");

    if (!printer_name.empty()) {
        lp_args.emplace_back("-d");
        lp_args.emplace_back(printer_name);
    }

    for (auto&& entry : _pp_queue)
        lp_args.emplace_back(entry.generated_file);

    // start lp process
    try {
        _lp_proc = ProcessRequest::create(lp_args, ProcessRequest::DEFAULT_TIMEOUT);
        _lp_proc->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionDataOut::on_lp_proc_finish));
        _lp_proc->start_process();
    } catch (const std::exception& ex) {
        auto xml_res = XmlResultInternalDeviceError::create(
            Glib::ustring::compose(
                "Starting printer command failed: %1", ex.what()));
        finished.emit(xml_res);
    }
}

void CoreFunctionDataOut::return_local_printer_result ()
{
    Glib::RefPtr <XmlResult> xml_res;
    std::vector <Glib::ustring> result_lines  { "<localPrinter>" };

    try {
	for (auto&& entry : _pp_queue) {
	    /* append contents of all generated Files
	     */
	    result_lines.push_back (FileHandler::get_file (entry.generated_file));
	}

	result_lines.push_back ("</localPrinter>");

	xml_res = XmlResultOk::create ("", result_lines);
    } catch (const std::exception& ex) {
        xml_res = XmlResultInternalDeviceError::create (
		Glib::ustring::compose ("Error reading generated Files: %1", ex.what ()));
    }

    finished.emit(xml_res);
}

void CoreFunctionDataOut::start_socket_copying()
{
    // start copy script process
    try {
        auto copy_script = get_copy_script();
        std::vector<std::string> copy_args(copy_script);

        _copy_proc = ProcessRequest::create(copy_args, ProcessRequest::DEFAULT_TIMEOUT);
        _copy_proc->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionDataOut::on_copy_proc_finish));
        _copy_proc->start_process();
    } catch (const std::exception& ex) {
        auto xml_res = XmlResultInternalDeviceError::create(
            Glib::ustring::compose(
                "Starting copy script command failed: %1", ex.what()));
        finished.emit(xml_res);
    }
}

void CoreFunctionDataOut::start_tar_process()
{
    // start tar process
    try {
        auto conf_handler = ConfHandler::get_instance();
        std::vector<std::string> tar_args{"tar", "zcvf"};
        std::vector<Glib::ustring> log_files;
        std::string serial = get_serial_number();
        std::string archive_name =
            serial + "__" + TimeUtilities::get_timestamp_log_format() + "." +
            _type + ".tar.gz";

        auto folder = conf_handler->getFolder(_dest, _type);
        if (folder.size() != 1)
            EXCEPTION("Failed to get conf folder from zixconf.xml");
	lHighDebug ("DataOut: starting tar process\n");
        std::string root_folder = conf_handler->getRootFolder(_dest);
        std::string conf_folder = folder[0].path();
        std::string path = root_folder + "/" + conf_folder + "/" + archive_name;

        tar_args.emplace_back(path);

        // get log files
        if (_type == "log")
            log_files = conf_handler->getAllLogFiles();
        else
            log_files = conf_handler->getAllMonitorFiles();
        for (auto&& file : log_files)
            tar_args.emplace_back(file);

        // start process
        _tar_proc = ProcessRequest::create(tar_args, ProcessRequest::DEFAULT_TIMEOUT);
        _tar_proc->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionDataOut::on_tar_proc_finish));
        _tar_proc->start_process();
    } catch (const std::exception& ex) {
        auto xml_res = XmlResultInternalDeviceError::create(
            Glib::ustring::compose(
                "Starting tar process failed: %1", ex.what()));
        finished.emit(xml_res);
    }
}

void CoreFunctionDataOut::start_copying()
{
    // file based destination? -> copy files
    if (_dest.is_file_based_dest()) {
        create_copy_queue();
        start_file_copying();
    }

    // printer? -> use `lp` command
    if (_dest.is_printer_dest()) {
        start_printer_copying();
    }

    // serial/socket based destination?
    if (_dest.is_socket_or_com_dest()) {
        start_socket_copying();
    }

    if (_dest.is_local_printer_dest ()) {
	return_local_printer_result ();
    }
}

void CoreFunctionDataOut::start_signature_creation(const std::string& xml_file)
{
    _sig_req = SignatureCreationRequest::create(xml_file, SignatureCreationRequest::MODE_HMAC);
    _sig_req->finished.connect(
        sigc::mem_fun(*this, &CoreFunctionDataOut::on_signature_creation_finish));
    _sig_req->start_creation();
}

void CoreFunctionDataOut::on_post_proc_finish(
    const Glib::RefPtr<ProcessResult>& result)
{
    std::string generated_file;

    // failure?
    if (!result->success()) {
	/* upon failure, we remove the file, we just tried to
	 * generate.
	 * Other Files generated earlier are valid, and can stay.
	 */
	const auto& entry = _pp_queue[_post_proc_idx];
	FileHandler::del_file (entry.generated_file);

	/* Create Error reply and return
	 */
        auto xml_res = XmlResultInternalDeviceError::create(
            Glib::ustring::compose("Postprocessor failed: %1",
                                   result->error_reason()));
        finished.emit(xml_res);
        return;
    }

    // next file to process? -> start next postprocessor call
    if (++_post_proc_idx < _pp_queue.size()) {
        try {
            const auto& entry = _pp_queue[_post_proc_idx];

	    /* save generated_file, becesue we might need to delete
	     * it in case of error
	     */
	    generated_file = entry.generated_file;

            _post_proc = ProcessRequest::create(
                { entry.post_processor, "--iid", _iid, "--format", entry.format },
                ProcessRequest::DEFAULT_TIMEOUT, false, entry.generated_file);
            _post_proc->finished.connect(
                sigc::mem_fun(*this, &CoreFunctionDataOut::on_post_proc_finish));
            if (!is_post_processing_needed(entry.generated_file))
                _post_proc->finished.emit(ProcessResult::create(0, "", "", true, 0));
            else
                _post_proc->start_process();
        } catch (const std::exception& ex) {
            auto xml_res = XmlResultInternalDeviceError::create(
                Glib::ustring(
                    "Couldn't start postprocessor script: %1", ex.what()));
	    if (!generated_file.empty ())
		FileHandler::del_file (generated_file);
            finished.emit(xml_res);
        }

        return;
    }

    // done, with post processing -> copy files to _dest
    start_copying();
}

void CoreFunctionDataOut::on_copy_finish(
    const Glib::RefPtr<CopyFileResult>& result)
{
    // failure?
    if (result->error()) {
        auto xml_res = XmlResultInternalDeviceError::create(
            Glib::ustring::compose("Copy failed: %1", result->error_msg()));
        finished.emit(xml_res);
        return;
    }

    // next file to copy? -> start next copy call
    if (++_copy_req_idx < _copy_queue.size()) {
        const auto& entry = _copy_queue[_copy_req_idx];

        _copy_req = CopyFileRequest::create(entry.src, entry.dest);
        _copy_req->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionDataOut::on_copy_finish));
        _copy_req->start_copy();

        return;
    }

    // done
    auto xml_res = XmlResultOk::create();
    finished.emit(xml_res);
}

void CoreFunctionDataOut::on_lp_proc_finish(
    const Glib::RefPtr<ProcessResult>& result)
{
    // failure?
    if (!result->success()) {
        auto xml_res = XmlResultInternalDeviceError::create(
            Glib::ustring::compose("Printer command failed: %1",
                                   result->error_reason()));
        finished.emit(xml_res);
        return;
    }

    // finished!
    auto xml_res = XmlResultOk::create();
    finished.emit(xml_res);
}

void CoreFunctionDataOut::on_copy_proc_finish(
    const Glib::RefPtr<ProcessResult>& result)
{
    // failure?
    if (!result->success()) {
        auto xml_res = XmlResultInternalDeviceError::create(
            Glib::ustring::compose("Copy script failed: %1",
                                   result->error_reason()));
        finished.emit(xml_res);
        return;
    }

    // finished!
    auto xml_res = XmlResultOk::create();
    finished.emit(xml_res);
}

void CoreFunctionDataOut::on_tar_proc_finish(
    const Glib::RefPtr<ProcessResult>& result)
{
    // failure?
    if (!result->get_exited_normally() ||
            (result->get_child_status() != 0 &&
             result->get_child_status() != 1)) {
        auto xml_res = XmlResultInternalDeviceError::create(
            Glib::ustring::compose("Tar process failed: %1",
                                   result->error_reason()));
        finished.emit(xml_res);
        return;
    }

    // finished!
    auto xml_res = XmlResultOk::create();
    finished.emit(xml_res);
}

void CoreFunctionDataOut::on_signature_creation_finish(
    const Glib::RefPtr<SignatureCreationResult>& result)
{
    if (!result->success()) {
        auto xml_res = XmlResultInternalDeviceError::create(
            Glib::ustring::compose(
                "Failed to create signature for config XML file: %1",
                result->error_msg()));
	/* when signature creation fails, we need to remove the xml file
	 */
	FileHandler::del_file(_xml_file);

        finished.emit(xml_res);
        return;
    }

    // finished!
    auto xml_res = XmlResultOk::create();
    finished.emit(xml_res);
}

void CoreFunctionDataOut::type_dispatcher()
{
    // 3 Cases: log/monitor ; measurement ; conf
    if (_type == "log" || _type == "monitor") {
        if (_dest.is_file_based_dest())
            start_tar_process();
        else if (_dest.is_socket_or_com_dest())
            start_socket_copying();
        else
            EXCEPTION("Printer is not supported for type " << _type);
    } else if (_type == "measurement") {
        // create postprocessing queue
        create_post_processing_queue();

        // start postprocessing
        start_postprocessing();
    } else if (_type == "conf") {
        // generate config xml file
        _xml_file = create_config_xml();

        // and start signature process
        start_signature_creation(_xml_file);
    } else {
        EXCEPTION("Unknown type: " << _type);
    }
}

void CoreFunctionDataOut::start_call()
{
    process_xml_params();

    if (!check_valid_xml()) {
        auto xml_res = XmlResultBadRequest::create("Received malformed XML");
        finished.emit(xml_res);
        return;
    }

    try {
        type_dispatcher();
    } catch (const std::exception& ex) {
        auto xml_res = XmlResultInternalDeviceError::create(ex.what());
        finished.emit(xml_res);
        return;
    }
}
