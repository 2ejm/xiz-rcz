
#ifndef ZIX_CORE_FUNCTION_PROCEDURE_H
#define ZIX_CORE_FUNCTION_PROCEDURE_H

#include "core_function_call.h"
#include "xml_parameter_list.h"
#include "xml_function.h"

#include "query_client.h"

#include "glibmm/refptr.h"

/**
 * \brief Procedure Function
 *
 * This function is called via <function fid="procedure">
 *
 * The Parameters to this function are also function calls,
 * these are executed in series...
 */
class CoreFunctionProcedure : public CoreFunctionCall
{
public:
    CoreFunctionProcedure (XmlParameterList parameters,
			   Glib::RefPtr <XmlDescription> description,
			   const Glib::ustring & textbody);

    ~CoreFunctionProcedure ();

    void start_call ();

    static Glib::RefPtr <CoreFunctionCall> factory (XmlParameterList parameters,
						    Glib::RefPtr <XmlDescription> description,
						    const Glib::ustring & textbody,
						    const xmlpp::Element * en);
private:
    std::list <Glib::RefPtr <XmlFunction> > _func_params;
    std::list <Glib::RefPtr <XmlFunction> >::iterator _func_param_iter;
    int _func_param_number;
    int _gui_param_number;
    int _retry;
    int _return_status;

    std::list <Glib::RefPtr <XmlResult> > _result_list;

    void proc_step_finished (Glib::RefPtr <XmlResult> result);
    void gui_query_finished (Glib::RefPtr <XmlResult> result);
    void dc_update_gui_restore_query_finished (Glib::RefPtr <XmlResult> result);
    void gui_finish_finished (Glib::RefPtr <XmlResult> result);

    void shutdown_procedure (int status);

    Glib::RefPtr <FunctionCall> _current_call;

    Glib::RefPtr <Query> _gui_query;
};

#endif
