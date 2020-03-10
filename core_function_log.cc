#include <stdexcept>

#include "core_function_log.h"
#include "xml_result_ok.h"
#include "xml_result_internal_device_error.h"
#include "log_handler.h"

CoreFunctionLog::CoreFunctionLog (XmlParameterList parameters,
				  Glib::RefPtr <XmlDescription> description,
				  const Glib::ustring & textbody)
    : CoreFunctionCall ("log", parameters, description, textbody)
{ }

CoreFunctionLog::~CoreFunctionLog ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionLog::factory (XmlParameterList parameters,
			  Glib::RefPtr <XmlDescription> description,
			  const Glib::ustring & textbody,
			  const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr <CoreFunctionCall> (new CoreFunctionLog (parameters,
								 description,
								 textbody));
}

void
CoreFunctionLog::start_call ()
{
    /*
     * Logging is done in a synchronious manner for now.
     */
    Glib::RefPtr<XmlResult> result = XmlResultOk::create();

    try {
        auto logger = LogHandler::get_instance();
        logger->log(_parameters);
    } catch (const std::exception& ex) {
        result = XmlResultInternalDeviceError::create(
            Glib::ustring::compose("Logging failed: '%1'", ex.what()));
    }

    call_finished (result);
}
