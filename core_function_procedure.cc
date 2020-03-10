
#include <stdexcept>
#include <cassert>

#include "core_function_procedure.h"
#include "xml_result_ok.h"
#include "xml_result_bad_request.h"
#include "xml_result_internal_device_error.h"
#include "log_handler.h"
#include "log.h"
#include "xml_function.h"
#include "xml_exception.h"
#include "xml_string_parameter.h"

#include "xml_query_gui_io.h"
#include "query_client.h"

#include "procedure_step_handler.h"


CoreFunctionProcedure::CoreFunctionProcedure (XmlParameterList parameters,
					     Glib::RefPtr <XmlDescription> description,
					     const Glib::ustring & textbody)
    : CoreFunctionCall ("procedure", parameters, description, textbody)
    , _func_params (_parameters.get_all <XmlFunction> ("function"))
    , _func_param_iter (_func_params.begin ())
    , _func_param_number (1)
    , _gui_param_number (1)
    , _retry (0)
    , _return_status (0)
{
    int step_int = get_current_proc_step ();

    if (step_int > 0) {
	/* save current retry count
	 */
	_retry = get_current_retry ();
	_result_list = get_current_result_list ();

	_retry += 1;
	note_current_proc_step (step_int, _retry);

	assert (_func_params.size () > (unsigned int) step_int);

	/* iterate over old parameters from before the reboot
	 */
	for (int i=1; i<step_int; i++) {
	    _func_param_iter++;
	    _func_param_number = i+1;

	    /* _gui_param_number is increased.
	     * it was initialised to 1 upon init
	     */
	    _gui_param_number += 1;

	    /* we might point behind the last step,
	     * need to check before we dereference the iter
	     */
	    if (_func_param_iter != _func_params.end ()) {

		/* to determin the correct _gui_param_number,
		 * we need to figuere out, whether _gui_param_number
		 * would have been reset during the reconstruction.
		 */
		Glib::RefPtr <XmlFunction> function = *_func_param_iter;

		if (function->get_fid () == "update") {
		    const XmlParameterList fparams = function->get_params ();

		    Glib::ustring target;

		    try {
			target = fparams.get_str ("target");
		    } catch (XmlException & e) {
			/* we got an exception while extracting the target.
			 * seems like target is not what we search for.
			 *
			 * leave it empty and just continue. we will just fall through.
			 */
		    }

		    if ((target == "DCBL") || (target == "DCF")) {
			/* that step was a DC update.
			 * also reset the _gui_param_number counter.
			 */

			_gui_param_number = 1;
		    }
		}
	    }
	}
    }
}

CoreFunctionProcedure::~CoreFunctionProcedure ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionProcedure::factory (XmlParameterList parameters,
			        Glib::RefPtr <XmlDescription> description,
			        const Glib::ustring & textbody,
				const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr <CoreFunctionCall> (new CoreFunctionProcedure (parameters,
								       description,
								       textbody));
}


void
CoreFunctionProcedure::proc_step_finished (Glib::RefPtr <XmlResult> result)
{
    lDebug ("CoreFunctionProcedure::proc_step_finished (result_status = %d)\n", result->get_status ());


    /* first of all, we remember the result
     */
    _result_list.push_back (result);

    if (result->get_status () != 200) {
	/* that step was an error
	 * we shutdown this procedure
	 */
	shutdown_procedure (500);
	return;
    }

    Glib::RefPtr <XmlFunction> function = *_func_param_iter;

    /* extra check for DC update step,
     * where we need to reset the gui
     */
    if (function->get_fid () == "update") {
	const XmlParameterList fparams = function->get_params ();

	Glib::ustring target;

	try {
	    target = fparams.get_str ("target");
	} catch (XmlException & e) {
	    /* we got an exception while extracting the target.
	     * seems like target is not what we search for.
	     *
	     * leave it empty and just continue. we will just fall through.
	     */
	}

	if ((target == "DCBL") || (target == "DCF")) {
	    /* that step was a DC update.
	     * lets get the gui in shape again
	     *
	     * also reset the _gui_param_number counter.
	     */

	    _gui_param_number = 1;

	    auto dc = QueryClient::get_instance ();
	    auto xq = XmlQueryGuiIO::create (_description, function->get_description ());
	    _gui_query = dc->create_query(xq);
	    _gui_query->finished.connect(
		sigc::mem_fun(*this, &CoreFunctionProcedure::dc_update_gui_restore_query_finished));
	    dc->execute(_gui_query);

	    return;
	}
    }

    /* now start with next step iteration
     */
    _func_param_iter++;
    _func_param_number++;
    _gui_param_number++;

    if (_func_param_iter != _func_params.end ()) {
	/* Note new step to execute
	 * and save it to file
	 */
	note_current_proc_step (_func_param_number, _result_list);

	/* look at next functiion
	 */
	function = *_func_param_iter;

	/* not having a descrition is not supported.
	 * create an error
	 */
        if (!function->get_description()) {
	    Glib::RefPtr <XmlResult> result = XmlResultBadRequest::create ("procedure function does not have a <description>");
	    _result_list.push_back (result);
	    shutdown_procedure (500);
	    return;
	}

	/* gui query
	 */
        auto dc = QueryClient::get_instance ();
        auto xq = XmlQueryGuiIO::create (function->get_description (), _gui_param_number);

        _gui_query = dc->create_query(xq);
        _gui_query->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionProcedure::gui_query_finished));
        dc->execute(_gui_query);
    } else {
	shutdown_procedure (200);
    }

}

void
CoreFunctionProcedure::shutdown_procedure (int status)
{
    lDebug ("CoreFunctionProcedure::shutdown_procedure (%d)\n", status);
    /* Note that the procedure finished
     * by setting current step to -1
     */
    note_current_proc_step (-1);
    /* If whole procedure was executed correctly,
     * increment exec attr
     */
    if(status == 200) {
        if (!increment_exec_attribute((
                        _parameters.get<XmlStringParameter>("id"))->get_str())){
            status=500;
            lDebug ("CoreFunctionProcedure::shutdown_procedure incrementing exec attr failed, setting status to 500");
        }
    }

    /* save the status, for the reply
     */
    _return_status = status;

    auto dc = QueryClient::get_instance ();
    auto xq = XmlQueryGuiIO::create ();

    _gui_query = dc->create_query(xq);
    _gui_query->finished.connect(
	    sigc::mem_fun(*this, &CoreFunctionProcedure::gui_finish_finished));
    dc->execute(_gui_query);
}

void
CoreFunctionProcedure::gui_finish_finished (Glib::RefPtr <XmlResult> result)
{
    (void) result;
    std::vector <Glib::ustring> result_strings;

    lDebug ("CoreFunctionProcedure::gui_finish_finished ()\n");

    for (auto r : _result_list) {
	result_strings.push_back (r->to_xml ());
    }

    Glib::RefPtr<XmlResult> res = XmlResult::create (_return_status, "", result_strings);
    call_finished (res);
}

void
CoreFunctionProcedure::dc_update_gui_restore_query_finished (Glib::RefPtr <XmlResult> result)
{
    (void) result;

    /* now start with next step iteration
     */
    _func_param_iter++;
    _func_param_number++;
    _gui_param_number++;

    if (_func_param_iter != _func_params.end ()) {
	/* Note new step to execute
	 * and save it to file
	 */
	note_current_proc_step (_func_param_number, _result_list);

	/* gui query
	 */
	auto function = *_func_param_iter;
        auto dc = QueryClient::get_instance ();
        auto xq = XmlQueryGuiIO::create (function->get_description (), _gui_param_number);

        _gui_query = dc->create_query(xq);
        _gui_query->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionProcedure::gui_query_finished));
        dc->execute(_gui_query);
    } else {
	shutdown_procedure (200);
    }
}

void
CoreFunctionProcedure::gui_query_finished (Glib::RefPtr <XmlResult> result)
{
    (void) result;

    lDebug ("CoreFunctionProcedure::gui_query_finished ()\n");

    auto function = *_func_param_iter;

    if (! function) {
	Glib::RefPtr<XmlResult> ret = XmlResultBadRequest::create ("wrong parameter");
	call_finished (ret);
	return;
    }

    try {
	/* we have started with step=0
	 * now, we notify ourselves, that we tried to start
	 */
	note_current_proc_step (_func_param_number, _retry);
	/* this is a procedure, we dont need to check
	 * the individual signatures
	 */
	_current_call = function->create_call (false);
	_current_call->finished.connect (sigc::mem_fun (*this, &CoreFunctionProcedure::proc_step_finished));
	_current_call->start_call ();
    } catch (const std::exception& e) {
	/* Failed to create the call,
	 * can be seen, when the individual proc_step is
	 * not allowed, for example
	 */

	Glib::RefPtr <XmlResult> result = XmlResultBadRequest::create (e.what());
	_result_list.push_back (result);
	shutdown_procedure (500);
	return;
    }
}

void
CoreFunctionProcedure::start_call ()
{
    lDebug ("CoreFunctionProcedure::start_call ()\n");
    /* first check, whether the retry count is too high
     */
    if (_retry > ZIX_PROC_STEP_MAX_RETRIES) {
	std::vector <Glib::ustring> result_strings;
	for (auto r : _result_list) {
	    result_strings.push_back (r->to_xml ());
	}

	note_current_proc_step (-1);

	Glib::RefPtr<XmlResult> res = XmlResultInternalDeviceError::create ("Too many Retries", result_strings);
	call_finished (res);
	return;
    }

    /* check if id given
     */
    const auto& id_param = _parameters.get<XmlStringParameter>("id");
    if (!id_param) {
        Glib::RefPtr<XmlResult> res = XmlResultBadRequest::create(
            "No ID parameter given for procedure");
        call_finished(res);
        return;
    }

    /* check if amount of executions is exceeded
     */
    const auto& exec_param = _parameters.get<XmlStringParameter>("exec");
    if (exec_param) {
        int exec;

        try {
            exec = std::stoi(exec_param->get_str());
        } catch (const std::exception& ex) {
            Glib::RefPtr<XmlResult> res = XmlResultBadRequest::create(
                "Exec attribute is not numeric");
            call_finished(res);
            return;
        }

        if (check_exec_exceeded(id_param->get_str(), exec)) {
            Glib::RefPtr<XmlResult> res = XmlResultBadRequest::create(
                Glib::ustring::compose("Exceeded exec attr for function id=%1",
                                       id_param->get_str()));
            call_finished(res);
            return;
        }
    }

    if (!_result_list.empty()) {
        if ( _result_list.back()->get_status() != 200) {
            lDebug ("CoreFunctionProcedure::start_call failure in reply status shuttingdown procedure\n");
            shutdown_procedure(500);
        }
    }

    if (!_description) {
	Glib::RefPtr<XmlResult> res = XmlResultBadRequest::create ("Missing Function description");
	call_finished (res);
	return;
    }

    /* now make sure, we are not at the end of the parameter_list
     */
    if (_func_param_iter != _func_params.end ()) {
	auto function = Glib::RefPtr <XmlFunction>::cast_dynamic (*_func_param_iter);

	/* make a sanity check, because we dont support empty descriptions,
	 * so we error out, when no description is set
	 */
        if (!function->get_description()) {
	    Glib::RefPtr <XmlResult> result = XmlResultBadRequest::create ("procedure function does not have a <description>");
	    _result_list.push_back (result);
	    shutdown_procedure (500);
	    return;
	}

        auto dc = QueryClient::get_instance ();

	/* what kind of query we emit depends on the step in the procedure.
	 * this is CoreFunctionProcedure::start_call (), but
	 * this is not necessarily step 1.
	 * We might be continuing after a reset
	 */
	Glib::RefPtr <XmlQuery> xq;

	if (_gui_param_number == 1)
	    xq = XmlQueryGuiIO::create (_description, function->get_description ());
	else
	    xq = XmlQueryGuiIO::create (function->get_description (), _gui_param_number);

        _gui_query = dc->create_query(xq);
        _gui_query->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionProcedure::gui_query_finished));
        dc->execute(_gui_query);
    } else if (_func_param_number==1) {
	// XXX: Empty Procedure... error ?
	Glib::RefPtr<XmlResult> result = XmlResultOk::create();
	call_finished (result);
    } else {
	/* When we just rebooted, and that procedure
	 * step was the last one, we come here.
	 *
	 * its all good, we shutdown_procedure with 200
	 */
	shutdown_procedure (200);
    }
}
