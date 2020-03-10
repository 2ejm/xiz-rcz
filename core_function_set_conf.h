#ifndef ZIX_CORE_FUNCTION_SET_CONF_H
#define ZIX_CORE_FUNCTION_SET_CONF_H
//-----------------------------------------------------------------------------
///
/// \brief  conf handler
///
///         see implemention for further details
///
/// \date   [20161129] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include "glibmm/refptr.h"

//---Own------------------------------

#include "core_function_call.h"
#include "xml_parameter_list.h"


//---Declaration---------------------------------------------------------------


class CoreFunctionSetConf : public CoreFunctionCall
{
    public:
    CoreFunctionSetConf (XmlParameterList parameters,
			 Glib::RefPtr <XmlDescription> description,
			 const Glib::ustring & textbody);

    ~CoreFunctionSetConf ();

	void start_call ();

	static Glib::RefPtr <CoreFunctionCall> factory (XmlParameterList parameters,
							Glib::RefPtr <XmlDescription> description,
							const Glib::ustring & textbody,
							const xmlpp::Element * en);
};


//---fin-----------------------------------------------------------------------
#endif // ! ? ZIX_CORE_FUNCTION_CONF_H
