#include <stdexcept>
#include <iostream>

#include "xml_string_parameter.h"
#include "xml_result_ok.h"
#include "xml_result_bad_request.h"
#include "xml_result_not_found.h"
#include "file_handler.h"
#include "id_mapper.h"
#include "utils.h"

#include "core_function_get_measurement.h"

CoreFunctionGetMeasurement::CoreFunctionGetMeasurement(
    XmlParameterList parameters,
    Glib::RefPtr <XmlDescription> description,
    const Glib::ustring & textbody) :
    CoreFunctionCall ("getMeasurement", parameters, description, textbody)
{ }

CoreFunctionGetMeasurement::~CoreFunctionGetMeasurement ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionGetMeasurement::factory(XmlParameterList parameters,
                                  Glib::RefPtr <XmlDescription> description,
				  const Glib::ustring & textbody,
				  const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr<CoreFunctionCall> (new CoreFunctionGetMeasurement
                                           (parameters,
                                            description,
					    textbody));
}

void CoreFunctionGetMeasurement::process_xml_params()
{
    PRINT_DEBUG ("CoreFunctionGetMeasurement::process_xml_params()");

    auto id_param     = _parameters.get<XmlStringParameter>("id");
    auto iid_param    = _parameters.get<XmlStringParameter>("iid");
    auto format_param = _parameters.get<XmlStringParameter>("format");
    _measurements     = _parameters.get_all<XmlMeasurement>("measurement");

    _id     = id_param     ? id_param->get_str()     : "";
    _iid    = iid_param    ? iid_param->get_str()    : "";
    _format = format_param ? format_param->get_str() : "";

    // a single measurement might be given by id/iid
    if (iid_param && _measurements.empty())
        _measurements.push_front(
            XmlMeasurement::create(_id, _iid, _format));
}

void CoreFunctionGetMeasurement::build_pp_queue()
{
    PRINT_DEBUG ("CoreFunctionGetMeasurement::build_pp_queue()");
    // for each measurement we get a queue -> "merge" it
    for (auto&& measurement : _measurements) {
        const auto& id     = _id.empty()  ? measurement->get_id()  : _id;
        auto iid           = _iid.empty() ? measurement->get_iid() : _iid;
        const auto& format = measurement->get_format().empty() ? _format : measurement->get_format();

	PRINT_DEBUG ("looking at measurement iid " << iid.raw ());
        if (iid.empty() && id.empty())
            EXCEPTION("Measurement cannot be determined, as no id/iid has been given");

        if (iid.empty()) {
            auto id_mapper = IdMapper::get_instance();
            iid = id_mapper->get_iid(id);
        }

        auto queue = PostProcessingBuilder().build_queue_without_dest(iid, format);

        // set id as well
        for (auto&& entry : queue)
            entry.id = id;

        _pp_queue.insert(_pp_queue.end(), queue.begin(), queue.end());
    }

    if (_pp_queue.empty())
        EXCEPTION("Failed to find measurements given by id/iid");
}

void CoreFunctionGetMeasurement::start_post_proc()
{
    const auto& entry = *_pp_it;

    _pp_proc = ProcessRequest::create(
        { entry.post_processor, "--iid", entry.iid, "--format", entry.format },
        ProcessRequest::DEFAULT_TIMEOUT, false, entry.generated_file);
    _pp_proc->finished.connect(
        sigc::mem_fun(*this, &CoreFunctionGetMeasurement::on_post_proc_finished));

    // skip if output file already exists
    if (FileHandler::file_exists(entry.generated_file)) {
        _pp_proc->finished.emit(ProcessResult::create(0, "", "", true, 0));
        return;
    }

    _pp_proc->start_process();
}

void CoreFunctionGetMeasurement::start_post_processing()
{
    _pp_it = _pp_queue.cbegin();

    start_post_proc();
}

void CoreFunctionGetMeasurement::on_post_proc_finished(
    const Glib::RefPtr<ProcessResult>& result)
{
    if (!result->success()) {
	/* post processor was not successful
	 * we need to remove the generated File
	 */
	const auto& entry = *_pp_it;
	FileHandler::del_file (entry.generated_file);

        XML_RESULT_INTERNAL_DEVICE_ERROR("Postprocessing failed: "
                                         << result->error_reason());
        return;
    }

    ++_pp_it;

    if (_pp_it != _pp_queue.cend()) {
        try {
            start_post_proc();
        } catch (const std::exception& ex) {
            XML_RESULT_INTERNAL_DEVICE_ERROR(ex.what());
        }

        return;
    }

    // done with post processing -> read generated files
    start_reading();
}

void CoreFunctionGetMeasurement::start_reading()
{
    PRINT_DEBUG ("CoreFunctionGetMeasurement::start_reading()");
    _pp_it = _pp_queue.cbegin();

    const auto& entry = *_pp_it;
    _read_req = ReadFileRequest::create(entry.generated_file);
    _read_req->finished.connect(
        sigc::mem_fun(*this, &CoreFunctionGetMeasurement::on_read_finished));
    _read_req->start_read();
}

Glib::ustring CoreFunctionGetMeasurement::build_result_xml(
    const PostProcessingEntry& entry, const std::string& content) const
{
    Glib::ustring ret;

    ret += "<measurement ";
    if (!entry.id.empty())
        ret += Glib::ustring::compose("id=\"%1\" ", entry.id);
    if (!entry.iid.empty())
        ret += Glib::ustring::compose("iid=\"%1\" ", entry.iid);
    if (!entry.format.empty())
        ret += Glib::ustring::compose("format=\"%1\" ", entry.format);
    if (!content.empty())
        ret += Glib::ustring::compose(">\n%1</measurement>\n", content);
    else
        ret += "/>\n";

    return ret;
}

void CoreFunctionGetMeasurement::on_read_finished(
    const Glib::RefPtr<ReadFileResult>& result)
{
    PRINT_DEBUG ("CoreFunctionGetMeasurement::on_read_finished()");
    if (result->error()) {
        XML_RESULT_NOT_FOUND("Read measurement failed: " << result->error_msg());
        return;
    }

    // save content
    const auto& entry       = *_pp_it;
    auto        encoded     = FileHandler::base64_encode(result->content());
    _return_values.emplace_back(build_result_xml(entry, encoded));

    // start with next file
    ++_pp_it;
    if (_pp_it != _pp_queue.cend()) {
        _read_req = ReadFileRequest::create(_pp_it->generated_file);
        _read_req->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionGetMeasurement::on_read_finished));
        _read_req->start_read();

        return;
    }

    // done!
    XML_RESULT_OK_RET("", _return_values);
}

void CoreFunctionGetMeasurement::start_call()
{
    process_xml_params();

    try {
        if (!_measurements.size())
            EXCEPTION("No measurements given!");

        build_pp_queue();

        start_post_processing();
    } catch (const std::exception& ex) {
        XML_RESULT_INTERNAL_DEVICE_ERROR(ex.what());
    }
}
