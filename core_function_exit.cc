#include <stdexcept>
#include <stdlib.h>

#include "core_function_exit.h"
#include "xml_result_ok.h"
#include "xml_result_internal_device_error.h"
#include "log_handler.h"

CoreFunctionExit::CoreFunctionExit (XmlParameterList parameters,
				  Glib::RefPtr <XmlDescription> description,
				  const Glib::ustring & textbody)
    : CoreFunctionCall ("exit", parameters, description, textbody)
{ }

CoreFunctionExit::~CoreFunctionExit ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionExit::factory (XmlParameterList parameters,
			  Glib::RefPtr <XmlDescription> description,
			  const Glib::ustring & textbody,
			  const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr <CoreFunctionCall> (new CoreFunctionExit (parameters,
								 description,
								 textbody));
}

Glib::RefPtr<Glib::MainLoop>
CoreFunctionExit::mainloop;

void
CoreFunctionExit::start_call ()
{
    Glib::RefPtr<XmlResult> result = XmlResultOk::create();

    call_finished (result);
    if (mainloop) {
	mainloop->quit();
    } else {
	exit (10);
    }
}
