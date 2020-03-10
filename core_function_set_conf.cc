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

#include <stdexcept>

//---Own------------------------------

#include "core_function_set_conf.h"
#include "xml_result_ok.h"
#include "xml_result_bad_request.h"
#include "xml_resource.h"
#include "xml_resource_parameter.h"
#include "xml_conf_parameter.h"
#include "xml_string_parameter.h"
#include "xml_exception.h"
#include "conf_handler.h"
#include "log.h"


//---Implementation------------------------------------------------------------


CoreFunctionSetConf::CoreFunctionSetConf (XmlParameterList parameters,
				  Glib::RefPtr <XmlDescription> description,
				  const Glib::ustring & textbody)
    : CoreFunctionCall ("setConf", parameters, description, textbody)
{ }


CoreFunctionSetConf::~CoreFunctionSetConf ()
{ }


Glib::RefPtr <CoreFunctionCall>
CoreFunctionSetConf::factory (XmlParameterList parameters,
			  Glib::RefPtr <XmlDescription> description,
			  const Glib::ustring & textbody,
			  const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr <CoreFunctionCall> (new CoreFunctionSetConf (parameters,
								 description,
								 textbody));
}

void
CoreFunctionSetConf::start_call ()
{
    auto parameters = _parameters.get_all<XmlConfParameter>("parameter");
    Glib::ustring mode;
    Glib::RefPtr<ConfHandler> conf=ConfHandler::get_instance();
    Glib::RefPtr<XmlResult> result=XmlResultOk::create();
    //bool erroneous=false;
    Glib::ustring node;

    // The mode parameter is optional
    try
    {
        mode=_parameters.get_str("mode");
    }
    catch( const XmlException& e )
    {
       mode="normal";
    }

    // The mode parameter is optional
    try
    {
        node=_parameters.get_str("node");
    }
    catch( const XmlException& e )
    {
    }

    lDebug ("setconf mode: %s\n", mode.c_str() );

    if(mode=="reset")
    {
        if( ! conf->resetConf(ZIX_CONFIG_DEFAULTS_FILENAME) )
        {
            result = XmlResultBadRequest::create("Could not restore factory defaults. Check the xml file.");
        }
    }
    // "node"-Mode
    else if( !node.empty() )
    {
	auto value = _parameters.get <XmlStringParameter> ("value");
	auto del   = _parameters.get <XmlStringParameter> ("delete");
	auto unit  = _parameters.get_str_default ("unit", "");

	if (value && del) {
            result = XmlResultBadRequest::create( "Value and delete parameter may not be set both at once." );
	    goto out;
	}

	if (value) {
	    if(!conf->setConfNode( node, value->get_str (), unit ) )
		result = XmlResultBadRequest::create( "SetConf failed, could not find paramter" );
	} else if (del) {
	    if (!conf->delConfNode (node))
		result = XmlResultBadRequest::create( "SetConf failed, could not find node to delete" );
	} else {
            result = XmlResultBadRequest::create( "Either value or delete parameter must be set." );
	}
    }
    else
    {
        try
        {
            // Iterate trough <resource> tags

            lDebug("setconf parameters count %d\n", parameters.size() );
            conf->clearConfigChangedHandlerMask();

            for (auto&& parameter : parameters) {
                Glib::ustring id = parameter->get_id();
                Glib::ustring value = parameter->get_value();
                Glib::ustring unit = parameter->get_unit();
                lDebug("setconf parameter ID:%s value:%s\n", id.c_str(), value.c_str() );
		if( ! parameter->has_value ())
                {
                    result = XmlResultBadRequest::create( "SetConf failed, no value" );
                    throw 1;
                }
                if (id.empty())
                {
                    result = XmlResultBadRequest::create("Id is empty in <files> tag");
                    throw 1;
                }
                else
                {
                    try {
                        if(!conf->setConfParameter( parameter->get_id()
                                      , parameter->get_unit(), parameter->get_value()
                                      ) )
                            result = XmlResultBadRequest::create( "SetConf failed, could not find paramter" );
                    }
                    catch ( ... ) {
                        result = XmlResultBadRequest::create( "SetConf failed, could not get parameter" );
                        throw 1;
                    }
                }
            }

            conf->emitConfigChanged();
        }
        catch (const std::exception& ex) {
            PRINT_ERROR("Failed to set config: " << ex.what());
        }
        catch( ... )
        {
            lError("Could not process setConf\n");
        }
    }
out:
    call_finished (result);
}


//---fin-----------------------------------------------------------------------
