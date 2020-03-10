
#ifndef ZIX_CORE_FUNCTION_LOG_H
#define ZIX_CORE_FUNCTION_LOG_H

#include "core_function_call.h"
#include "xml_parameter_list.h"

#include "glibmm/refptr.h"

/**
 * \brief Logging Function
 *
 * This function is called via <function fid="log">
 *
 * Currently only a Prototype calling printf
 */
class CoreFunctionLog : public CoreFunctionCall
{
public:
    CoreFunctionLog (XmlParameterList parameters,
             Glib::RefPtr <XmlDescription> description,
	     const Glib::ustring & textbody);

    ~CoreFunctionLog ();

    void start_call ();

    static Glib::RefPtr <CoreFunctionCall> factory (XmlParameterList parameters,
						    Glib::RefPtr <XmlDescription> description,
						    const Glib::ustring & textbody,
						    const xmlpp::Element * en);
};

#endif
