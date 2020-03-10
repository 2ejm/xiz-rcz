
#ifndef ZIX_CORE_FUNCTION_EXIT_H
#define ZIX_CORE_FUNCTION_EXIT_H

#include "core_function_call.h"
#include "xml_parameter_list.h"

#include <glibmm/refptr.h>
#include <glibmm/main.h>


/**
 * \brief Exit Function
 *
 * This function is called via <function fid="exit">
 *
 * Its for debugging purposes
 */
class CoreFunctionExit : public CoreFunctionCall
{
public:
    static Glib::RefPtr<Glib::MainLoop> mainloop;

    CoreFunctionExit (XmlParameterList parameters,
             Glib::RefPtr <XmlDescription> description,
	     const Glib::ustring & textbody);

    ~CoreFunctionExit ();

    void start_call ();

    static Glib::RefPtr <CoreFunctionCall> factory (XmlParameterList parameters,
						    Glib::RefPtr <XmlDescription> description,
						    const Glib::ustring & textbody,
						    const xmlpp::Element * en);
};

#endif
