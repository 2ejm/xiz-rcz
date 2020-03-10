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

#include "core_function_get_conf.h"
#include "xml_result_ok.h"
#include "xml_resource.h"
#include "xml_result_bad_request.h"
#include "conf_handler.h"
#include "log.h"
#include "xml_string_parameter.h"


//---Implementation------------------------------------------------------------


CoreFunctionGetConf::CoreFunctionGetConf (XmlParameterList parameters,
				  Glib::RefPtr <XmlDescription> description,
				  const Glib::ustring & textbody)
    : CoreFunctionCall ("getConf", parameters, description, textbody)
{ }


CoreFunctionGetConf::~CoreFunctionGetConf ()
{ }


Glib::RefPtr <CoreFunctionCall>
CoreFunctionGetConf::factory (XmlParameterList parameters,
			  Glib::RefPtr <XmlDescription> description,
			  const Glib::ustring & textbody,
			  const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr <CoreFunctionCall> (new CoreFunctionGetConf (parameters,
								 description,
								 textbody));
}

void
CoreFunctionGetConf::start_call ()
{
    auto parameters = _parameters.get_all<XmlResource>("parameters");
    //Glib::ustring output;
    Glib::RefPtr<ConfHandler> conf=ConfHandler::get_instance();
    bool done=false;
    Glib::RefPtr<XmlResult> result;
    std::vector<Glib::ustring> resultStrings;

    // check for so called "node"
    try
    {
        Glib::ustring node;
        try { node=_parameters.get_str("node"); } catch ( ... ){};
        if(!node.empty())
        {
            lDebug ("Reading Node");
            resultStrings.push_back( conf->getConfNode(node) );
            done=true;
        }
    }
    catch( std::exception & e )
    {
        result= XmlResultBadRequest::create( e.what() );
        call_finished (result);
        return;
    }

    if(!done)
        try
        {
            Glib::ustring item, id, resource, postprocessor;
            try{ item=_parameters.get_str("item"); } catch ( ... ){};
            try{ id=_parameters.get_str("id"); } catch ( ... ){};
            try{ resource=_parameters.get_str("resource"); } catch ( ... ){};
            try{ postprocessor=_parameters.get_str("postProcessor"); } catch ( ... ){};

            resultStrings.push_back( conf->getConf( item, id, resource, postprocessor) );
            done=true;
        }
        catch( ... )
        {
            result= XmlResultBadRequest::create( "Item not found" );
            call_finished (result);
            return;
        }

    // No resource/id was given? fall back to default "all"
    if( !done )
    {
        lError("Nothing to do\n");
        result= XmlResultBadRequest::create( "parameter missing" );
    }
    else
    {
        result= XmlResultOk::create("", resultStrings);
    }

    call_finished (result);
}


//---fin-----------------------------------------------------------------------
