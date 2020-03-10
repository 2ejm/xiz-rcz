#include <stdexcept>
#include <iostream>
#include <vector>

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
#include "utils.h"

#include "core_function_data_in.h"

CoreFunctionDataIn::CoreFunctionDataIn(
    XmlParameterList parameters,
    Glib::RefPtr <XmlDescription> description,
    const Glib::ustring& text)
    : CoreFunctionCall ("dataIn", parameters, description, text)
{ }

CoreFunctionDataIn::~CoreFunctionDataIn ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionDataIn::factory(XmlParameterList parameters,
                              Glib::RefPtr <XmlDescription> description,
                              const Glib::ustring & text,
			      const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr<CoreFunctionCall>(
        new CoreFunctionDataIn(parameters, description, text));
}

bool CoreFunctionDataIn::check_xml_and_get_values()
{
    const auto& src_param  = _parameters.get<XmlStringParameter>("src");
    const auto& type_param = _parameters.get<XmlStringParameter>("type");

    _src  = src_param  ? src_param->get_str()  : "";
    _type = type_param ? type_param->get_str() : "";

    if (_src.empty())
        return false;

    if (_type.empty())
        return false;

    return true;
}

std::string CoreFunctionDataIn::get_input_processor() const
{
    auto conf_handler = ConfHandler::get_instance();
    auto ip = conf_handler->getInputProcessorByType(_type);

    return ip.code();
}

void CoreFunctionDataIn::on_ip_proc_finish(const Glib::RefPtr<ProcessResult>& result)
{
    if (!result->success()) {
        XML_RESULT_INTERNAL_DEVICE_ERROR("Input processor failed: " << result->error_reason());
        return;
    }

    if (! ConfHandler::get_instance()->resetConf(ZIX_CONFIG_NEW_FILENAME)) {
	/* failed to read config
	 * remove the broken config anyways, and then report error
	 */

	FileHandler::del_file (ZIX_CONFIG_NEW_FILENAME);
        XML_RESULT_INTERNAL_DEVICE_ERROR("Failed to load " << ZIX_CONFIG_NEW_FILENAME);
	return;
    }

    /* config has been update successfully.
     * remove ZIX_CONFIG_NEW_FILENAME and report sucess
     */
    FileHandler::del_file (ZIX_CONFIG_NEW_FILENAME);

    XML_RESULT_OK_RET (result->stderr (), {result->stdout ()});
}

void CoreFunctionDataIn::start_ip()
{
    const auto ip_script = get_input_processor();
    if (ip_script.empty())
        EXCEPTION("Couldn't find input processor for type " << _type);

    _ip_proc = ProcessRequest::create(
        { ip_script, "--src", _src }, ProcessRequest::DEFAULT_TIMEOUT, true, "", true);
    _ip_proc->finished.connect(
        sigc::mem_fun(*this, &CoreFunctionDataIn::on_ip_proc_finish));
    _ip_proc->start_process();
}

void CoreFunctionDataIn::start_call()
{
    std::string dir;

    if (!check_xml_and_get_values()) {
        XML_RESULT_BAD_REQUEST("Malformed XML received");
        return;
    }

    try {
        start_ip();
    } catch (const std::exception& ex) {
        XML_RESULT_INTERNAL_DEVICE_ERROR(ex.what());
        return;
    }
}
