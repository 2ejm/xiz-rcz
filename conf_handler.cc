//-----------------------------------------------------------------------------
///
/// \brief  conf handler
///
///         setConf
///         getConf
///
/// \date   [20161129] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//---General--------------------------

#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "ext/stdio_filebuf.h"

//---Own------------------------------

#include "conf_handler.h"
#include "xml_parameter_list.h"
#include "xml_helpers.h"
#include "time_utilities.h"
#include "log.h"
#include "watchdog_manager.h"
#include "samba_mounter.h"
#include "simple_crypt.h"
#include "network_config.h"


#include <libxml++/nodes/element.h>
#include <libxml++/nodes/node.h>
#include <libxml++/libxml++.h>
#include <libxml/tree.h>


//---Implementation------------------------------------------------------------



//ConfHandler *ConfHandler::instance=NULL;
Glib::RefPtr<ConfHandler> ConfHandler::instance;

Glib::RefPtr<ConfHandler>
ConfHandler::get_instance()
{
    if (!instance)
	instance = create();

    return instance;
};


/// \brief  Le constructeur
ConfHandler::ConfHandler()
{
    load();

    language=getConfParameter("language");
}


/// \brief  Le destructeur
ConfHandler::~ConfHandler()
{
}


Glib::RefPtr<ConfHandler>ConfHandler::create() /* static */
{
    return Glib::RefPtr<ConfHandler>( new ConfHandler );
}


void ConfHandler::initXmlDocument()
{
    // Have to parse something in order to get document
    parser.parse_memory("<?xml version=\"1.0\" encoding=\"UTF-8\"?><sic></sic>");
    xmlpp::Document *doc=parser.get_document();
    if(!doc)
        throw("Could not initialize xml document");
    nodeRoot = parser.get_document()->create_root_node( ZIX_CONFIG_ROOT_NODE );
}


void ConfHandler::parameterGotUpdated(   const Glib::ustring &parameterId
                              , const Glib::ustring &parameterValue )
{
    PRINT_DEBUG ("ConfHandler::parameterGotUpdated() " << parameterId.raw () << " = " << parameterValue.raw ());
    save();
    confChangeAnnounce.emit( parameterId, parameterValue, configChangedHandlerMask );

    if( parameterId == "language")
        language=parameterValue;

    return;
}

void ConfHandler::emitConfigChanged( )
{
    confChanged.emit( configChangedHandlerMask );
    return;
}


bool ConfHandler::setConfParameter(const Glib::ustring& paramId
                          , const Glib::ustring& unit, const Glib::ustring& value )
{
    PRINT_DEBUG ("ConfHandler::setConfParameter () " << paramId.raw () << " " << value.raw ());
    xmlpp::Element* element;
    xmlpp::Node* node;

    // try to set date or time
    if (paramId == "date")
    {
        lDebug("setDate\n");
        return TimeUtilities::set_date(value);
    }
    if (paramId == "time")
    {
        lDebug("setTime\n");
        return TimeUtilities::set_time(value);
    }

    // reconfigure network if host name is set
    if (paramId == "deviceHostname") {
        auto net = CNetworkConfig::get_instance();
        net->collectData();
        net->updateNetwork();
    }

    PRINT_DEBUG ("ConfHandler::setConfParameter() " << __LINE__);
    Glib::ustring query = Glib::ustring::compose("/" ZIX_CONFIG_ROOT_NODE "//%1[@id='%2']", "parameter", paramId);

    node=findElement(nodeRoot, Glib::ustring::compose("/" ZIX_CONFIG_ROOT_NODE "//%1[@id='%2']", "parameter", paramId));
    element = dynamic_cast<xmlpp::Element *>( node );
    //element=findCreateElement(element, "%1[@id='%2']", "parameter", param);
    if(!element)
    {
        lError("Could not find node to set: %s", paramId.c_str() );
        return(false);
    }

    if ((element->get_attribute_value ("value") == value) && (element->get_attribute_value ("unit") == unit)) {
	/* no need to set anything, its already
	 * setup. Just return
	 */
	return true;
    }

    PRINT_DEBUG ("ConfHandler::setConfParameter() " << __LINE__);
    element->set_attribute("value", value);
    if( ! unit.empty() )
        element->set_attribute("unit", unit);

    //PRINT_DEBUG ("element : " << node_to_string (nodeRoot, 0, 0));
    PRINT_DEBUG ("ConfHandler::setConfParameter() " << __LINE__);
    parameterGotUpdated( paramId , value);
    PRINT_DEBUG ("ConfHandler::setConfParameter() " << __LINE__);

    return(true);
}


bool ConfHandler::setConfNode( const Glib::ustring& node, const Glib::ustring& value, const Glib::ustring& unit)
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Element *element;
    xmlpp::Node *startNode=nodeRoot;
    bool done=false;
    int changes=0;
    Glib::ustring attribute;

    lDebug( "Node\n" );
    //nodeSet=startNode->findCreateElement( node /*Glib::ustring::compose("node, resource )*/ );
    findCreateElement( startNode, node , changes);

    // If nodes have been created save the file
    if(changes)
        save();


    nodeSet=startNode->find( node );

    if(nodeSet.size()==0)
    {
        lError("Could not find node for setConfNode\n");
        return(done);
    }

    for (auto&& resource : nodeSet)
    {
        element= dynamic_cast<xmlpp::Element *>( resource );
        if(element)
        {
            if (! attribute.empty() )
                element->set_attribute(attribute,value);
            else
                element->set_child_text(value);
            if( ! unit.empty() )
                element->set_attribute("unit",unit);

            parameterGotUpdated( element->get_name(), value );

            done=true;
        }
        else {
            auto attribute = dynamic_cast<xmlpp::Attribute *>( resource );
            if (attribute)
            {
                attribute->set_value(value);
                parameterGotUpdated( attribute->get_name(), value );
                done=true;
            }
        }
        // Onyl one node when in this mode
        break;
    }

    return(done);
}

bool ConfHandler::delConfNode (const Glib::ustring & node)
{
    /* first find nodes
     */
    xmlpp::NodeSet nodeSet = nodeRoot->find (node);

    /* check if we have some nodes
     */
    if (nodeSet.size()==0) {
        lError("ConfHandler::delConfNode (): Could not find node to delete \n");
	return false;
    }

    /* iterate over nodes, find parent, and remove
     */
    for (auto node : nodeSet) {
	auto parent = node->get_parent ();

	if (parent) {
	    parent->remove_child (node);
	} else {
	    lError ("Failed to Remove Node without parent");
	}
    }

    /* save Config, after deleteing nodes
     * we save in any case, even if something went wrong
     */
    save ();

    return true;
}

xmlpp::Node *ConfHandler::findCreateElement(
        xmlpp::Node *node, const Glib::ustring &xpath
        , const Glib::ustring &name, const Glib::ustring &id
        )
{
    xmlpp::Element *element;

    xmlpp::NodeSet nodeSet=node->find(
                Glib::ustring::compose(xpath, name, id )
    );

    if(!nodeSet.size())
    {
        element=node->add_child(name);
        element->set_attribute("id", id);
        node = dynamic_cast<xmlpp::Node *>( element );
    }
    else
    {
        node = nodeSet.front();
        if(!node)
            throw std::logic_error("node is not a element\n");
    }

    return node;
}


xmlpp::Node *ConfHandler::findCreateElement(
        xmlpp::Node *node, const Glib::ustring &xpath, int &changes )
{
    xmlpp::Element *element= nullptr;

    if( (xpath=="./") || (xpath==".//") )
    {
        return node;
    }

    xmlpp::NodeSet nodeSet=node->find( xpath );

    if(!nodeSet.size())
    {
        int pos=xpath.find_last_of("/");
        xmlpp::Element *preelement;

        // No more subnotes?
        if(pos<=0)
        {
            return node;
        }

        lHighDebug("Find sub string: %s\n", xpath.substr(0, pos).c_str() );
        auto prenode=findCreateElement( node, xpath.substr(0, pos), changes);
        preelement= dynamic_cast<xmlpp::Element *>( prenode );

        lHighDebug("Found: %s\n", preelement->get_name().c_str() );
        if(preelement)
        {
            Glib::ustring child=xpath.substr(pos+1);
            Glib::ustring childname;
            Glib::ustring attribute;
            Glib::ustring value;
            int subpos=child.find_first_of("[");
            int endpos;
            int eqpos;

            //if(subpos>=0)
            childname=child.substr(0, (subpos>=0)?subpos:Glib::ustring::npos);

            lHighDebug("Create child: %s\n", childname.c_str() );
            element=preelement->add_child( childname );
            changes++;

            while( (subpos=child.find_first_of("[")) >= 0 )
            {

                endpos=child.find_first_of("]");

                if( (child[subpos+1]!='@') || (endpos<=0) )
                    throw std::logic_error("Can not parse xpath (1)");

                attribute=child.substr(subpos+2, endpos-(subpos+2));

                eqpos=attribute.find_first_of("=");
                if(  (eqpos<=0) )
                    throw std::logic_error("Can not parse xpath (2)");

                value=attribute.substr(eqpos+1);
                attribute=attribute.substr(0, eqpos);

                // Cut Quotes?
                if( ( value[0] ==  '\'') || ( value[0] ==  '"') )
                {
                    value=value.substr(1, value.length()-2);
                }

                lHighDebug("Create atribuge: %s = %s\n", attribute.c_str(), value.c_str() );
                element->set_attribute(attribute, value);

                child=child.substr(endpos+1);
            }
        }
        //element->set_attribute("id", id);
        node = dynamic_cast<xmlpp::Node *>( element );
    }
    else
    {
        node = nodeSet.front();
        if(!node)
            throw std::logic_error("node is not a element\n");
    }

    return node;
}


xmlpp::Element *ConfHandler::findElement(
        xmlpp::Node *node, const Glib::ustring &path )
{
    xmlpp::Element *element=nullptr;

    xmlpp::NodeSet nodeSet=node->find(path);

    if(!nodeSet.size())
    {
        element=nullptr;
    }
    else
    {
        element= dynamic_cast<xmlpp::Element *>( nodeSet.front() );
        if(!element)
            throw std::logic_error("node is not a element\n");
    }

    return element;
}


xmlpp::Element *ConfHandler::findElement( const Glib::ustring &path )
{
        return(findElement(nodeRoot, path));
}


xmlpp::NodeSet ConfHandler::getConfXmlByName(const Glib::ustring& item, xmlpp::Node* start /*=NULL*/ )
{
    xmlpp::NodeSet nodeSet;

    if(!start)
        start=nodeRoot;

    nodeSet=start->find(
                            Glib::ustring::compose(
                                        ".//%1"
                                        , item)
                            );
    return(nodeSet);
}


bool ConfHandler::getConf(const Glib::ustring& param
                          , Glib::ustring& unit, Glib::ustring& value )
{
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    xmlpp::NodeSet nodeSet;


    nodeSet=nodeRoot->find(
                            Glib::ustring::compose(
                                        "/" ZIX_CONFIG_ROOT_NODE "//parameter[@id='%1']"
                                        , param)
                            );

    // Must clear them anyway because attributes might be missing
    value.clear();
    unit.clear();

    if(nodeSet.size())
    {
        element= dynamic_cast<xmlpp::Element *>( nodeSet.front() );
        if(!element)
            throw std::logic_error("node is not a element\n");

        if ( ( attribute=element->get_attribute("unit") ) )
            unit=attribute->get_value();
        if ( ( attribute=element->get_attribute("value") ) )
            value=attribute->get_value();
        return( true );
    }

    return(false);
}


Glib::ustring ConfHandler::getConfParameter(const Glib::ustring& param)
{
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    xmlpp::NodeSet nodeSet;

    nodeSet=nodeRoot->find(
                            Glib::ustring::compose(
                                        "/" ZIX_CONFIG_ROOT_NODE "//parameter[@id='%1']"
                                        , param)
                            );

    if(nodeSet.size())
    {
        element= dynamic_cast<xmlpp::Element *>( nodeSet.front() );
        if(!element)
        {
            throw std::logic_error("node is not a element\n");
        }
        attribute=element->get_attribute("value");
        if(attribute)
            return( attribute->get_value() );
    }

    return( Glib::ustring() );
}


Glib::ustring ConfHandler::getConf(const Glib::ustring& item, const Glib::ustring& id
                          , const Glib::ustring& resource, const Glib::ustring& postProcessor)
{
    if ( item == "date" )
        return( getConfDate() );

    if ( item == "time" )
        return( getConfTime() );

    if ( ( item == "parameter" ) && ( id == "date") )
        return( getConfDate() );

    if ( ( item == "parameter" ) && ( id == "time") )
        return( getConfTime() );

    if ( ( item == "format" ) && ( ! resource.empty() ) )
        return( getConfFormatResource( resource ) );

    return( getConfRegular( item, id, resource, postProcessor) );
}


Glib::ustring ConfHandler::getConfRegular(const Glib::ustring& item, const Glib::ustring& id
                          , const Glib::ustring& resource, const Glib::ustring& postProcessor)
{
    xmlpp::NodeSet nodeSet;
    //xmlpp::NodeSet nodeSetFinal;
    xmlpp::NodeSet nodeSetTemp;
    xmlpp::Element *element;
    Glib::ustring text;
    xmlpp::Node *startNode=nodeRoot;
    bool collected=false;

    lHighDebug("Dump getConfRegular(): %s\n", element_to_string((xmlpp::Element *)startNode, 0, 1).c_str() );

    if(!resource.empty())
    {
        nodeSet=startNode->find( Glib::ustring::compose(".//interface/resource[@id='%1']", resource ) );
        lDebug( "Resource: %s: %d\n", resource.c_str(), nodeSet.size() );
        if(nodeSet.size()==0)
        {
            throw std::logic_error( "getConf: cant find resource" );
        }
        startNode=nodeSet.at(0);
        collected=true;
    }

    if(!postProcessor.empty())
    {
        nodeSet=startNode->find( Glib::ustring::compose(".//postProcessing/postProcessor[@id='%1']", postProcessor ) );
        lDebug( "Postprocessor: %s: %d\n", postProcessor.c_str(), nodeSet.size() );
        if(nodeSet.size()==0)
            throw std::logic_error( "getConf: cant find postprocessor" );
        startNode=nodeSet.at(0);
        collected=true;
    }

    // We have not choosen any nodes yet; Just take all...
    if( !collected )
    {
        //nodeSet=startNode->find( ".//*" );
        nodeSet.push_back( nodeRoot );
        collected=true;
    }

    // Get all parameters for a given resource.
    // The resource "all" is a special resource which iterates through all resources
    for (auto&& resource : nodeSet)
    {
        element= dynamic_cast<xmlpp::Element *>( resource );
        if(element)
        {
            nodeSetTemp=element->find(
                        id.empty()
                            ? ( item.empty() ?
                                        ".//*":
                                              Glib::ustring::compose(
                                        ".//%1", item)
                              )
                            : Glib::ustring::compose(
                                        ".//%1[@id='%2']"
                                        , item, id)
                                    );

            if(nodeSetTemp.size()==0)
                throw std::logic_error( "getConf: cant find item" );

            for (auto&& resource : nodeSetTemp)
            {
                try
                {
                    hoistName( resource );
                }
                catch( ... )
                {
                    // Could not get language; ignoring...
                }

                element= dynamic_cast<xmlpp::Element *>( resource );
                if(element)
                {
		    if( item == "format" )
			text += element_to_string_no_recurse (element);
                    else if (item == "protocol")
                        text += element_to_string_recurse_one (element);
		    else
			text += element_to_string (element, 0, 0);
                }
                else
                    lWarn( "Could not find element\n" );
            }
        }
    }

    return(text);
}


Glib::ustring ConfHandler::getConfFormatResource( const Glib::ustring& resource )
{
    xmlpp::NodeSet nodeSet;
    xmlpp::NodeSet nodeSetLanguage;
    xmlpp::NodeSet nodeSetTemp;
    xmlpp::Element *element;
    Glib::ustring text;
    xmlpp::Node *startNode=nodeRoot;

    lHighDebug("Dump getConfFormatResource(): %s\n", element_to_string((xmlpp::Element *)startNode, 0, 0).c_str() );

    nodeSet=startNode->find( Glib::ustring::compose(".//interface/resource[@id='%1']/postProcessor", resource ) );
    lDebug( "Resource (getConfFormatResource): %d\n", nodeSet.size() );

    if(nodeSet.size()==0)
        throw std::logic_error( "getConf: cant find resource" );

    for (auto&& resource : nodeSet)
    {
        element= dynamic_cast<xmlpp::Element *>( resource );
        if(element)
        {
            nodeSetTemp=startNode->find(
                            Glib::ustring::compose(
                                        ".//postProcessing/postProcessor[@id='%1']/format"
                                        , element->get_attribute_value("id"))
                                    );
            for (auto&& resource : nodeSetTemp)
            {
                hoistName( resource );

                element= dynamic_cast<xmlpp::Element *>( resource );

                if(element)
                {
		    text += element_to_string_no_recurse (element);
                }
            }

        }
    }

    return(text);
}


Glib::ustring ConfHandler::getConfNode( const Glib::ustring& node )
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Element *element;
    xmlpp::Attribute *attribute;
    Glib::ustring text;
    xmlpp::Node *startNode=nodeRoot;
    xmlpp::Node *resource;

    lHighDebug("Dump getConfNode(): %s\n", element_to_string((xmlpp::Element *)startNode, 0, 0).c_str() );

    lDebug( "Node\n" );
    nodeSet=startNode->find( node /*Glib::ustring::compose("node, resource )*/ );
    if(nodeSet.size() > 1)
        throw std::logic_error( "Multiple Nodes found - XPATH ambiguous" );

    if(nodeSet.size() < 1)
        throw std::logic_error( "Node not found" );

    resource = nodeSet[0];

    hoistName( resource );

    element= dynamic_cast<xmlpp::Element *>( resource );
    attribute = dynamic_cast<xmlpp::Attribute *>( resource );

    if(element)
        text+=element_to_string( element, 0, 1 );

    if (attribute) {
        text += "<parameter id=\"";
        text += node;
        text += "\" value=\"";
        text += attribute->get_value();
        text += "\" />";
    }

    if (text.empty())
        throw std::logic_error("could not convert node to element");

    return (text);
}


Glib::ustring ConfHandler::getConfDate(  )
{
    xmlpp::Document *doc=new xmlpp::Document;
    xmlpp::Element *element;
    Glib::ustring text;

    Glib::DateTime date=Glib::DateTime::create_now_utc( );

    element=doc->create_root_node("parameter");
    element->set_attribute("id", "date");
    element->set_attribute("value", date.format("%F") );

    text+=element_to_string( element, 0, 1 );

    return( text );
}


Glib::ustring ConfHandler::getConfTime(  )
{
    xmlpp::Document *doc=new xmlpp::Document;
    xmlpp::Element *element;
    Glib::ustring text;

    Glib::DateTime date=Glib::DateTime::create_now_utc( );

    element=doc->create_root_node("parameter");
    element->set_attribute("id", "time");
    element->set_attribute("value", date.format("%T") );

    text+=element_to_string( element, 0, 1 );

    return( text );
}


bool ConfHandler::hoistName( xmlpp::Node *node )
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Element *element;
    xmlpp::Node::NodeList childrenList;

    nodeSet=node->find( Glib::ustring::compose(
                            ".//name[@language='%1']", getLanguage() ) );
    element= dynamic_cast<xmlpp::Element *>( node );

    // No translation available? use default entry
    if( !nodeSet.size() )
        nodeSet=node->find( ".//name[@default='1']" );

    if(nodeSet.size() && element)
    {
        xmlpp::Element *languageElement=dynamic_cast<xmlpp::Element *> ( nodeSet.front() );
        if(languageElement) {
	    xmlpp::TextNode * ct = languageElement->get_child_text();

	    if (ct) {
		element->set_attribute("name", ct->get_content() );
	    } else {
		Glib::ustring value = languageElement->get_attribute_value ("value");
		if (!value.empty ())
		    element->set_attribute("name", value );
		else
		    lError ("Language node does not have content nor value attribute\n");
	    }
	} else {
            lError("Could not cast language node.\n");
	}
    }

    if(element)
    {
        childrenList=element->get_children();
        for (auto&& child : childrenList)
        {
            if( child->get_name() == "name")
            {
                //delete child;
            }
            else
                hoistName( child );
        }
    }

    return( true );
}


bool ConfHandler::resetConf(const Glib::ustring & fname)
{
    bool success=false;

    try
    {
        parser.parse_file( fname );
        nodeRoot=parser.get_document()->get_root_node();
        success=true;
    }
    catch( ... )
    {
        lDebug("### Could not read xml file \"%s\"\n", fname.c_str());
        load();
    }

    parameterGotUpdated("all", "");

    return( success );
}


void ConfHandler::load()
{
    static const char* filenames[]=
    {
        ZIX_CONFIG_DAMAGED_FILENAME,
        ZIX_CONFIG_FILENAME,
        ZIX_CONFIG_BACKUP_FILENAME,
        ZIX_CONFIG_DEFAULTS_FILENAME,
        NULL
    };
    int i1=0;
    bool success=false;


    while( (!success) && filenames[i1] )
    {
        try
        {
            parser.parse_file( filenames[i1] );
            nodeRoot=parser.get_document()->get_root_node();
            success=true;
        }
        catch( ... )
        {
        }
        i1++;
    }

    if(!success)
    {
        lDebug("### Could not read ANY config file; \n");
        initXmlDocument();
    }

    /* after the xml is loaded from disk,
     * we need to decrypt the passwords
     */
    decrypt_passwords ();

    return;
}


void ConfHandler::save()
{
    PRINT_DEBUG ("ConfHandler::save()");

    /* because we want to call fsync (fd),
     * we need to jump through a few hoops now.
     *
     * use open to get an fd.
     */

    int fd = open (ZIX_CONFIG_TEMP_FILENAME, O_CREAT | O_WRONLY, 0644);

    if (fd < 0) {
	lError ("unable to open tmp config file for writing\n");
	return;
    }

    /* now we create some c++ stream...
     * through some gcc extension filebuf thingy
     */

    __gnu_cxx::stdio_filebuf<char> fbuf (fd, std::ios_base::out);
    std::ostream temp_stream (&fbuf);

    /* before writing data to disk,
     * we crypt the passwords inside the DOM
     */
    crypt_passwords ();

    /* do what we actually want,
     * write xml to file
     */
    parser.get_document()->write_to_stream (temp_stream);

    /* DOM is written to disk, decrypt passwords before doing anything else
     */
    decrypt_passwords ();

    /* now call flush, and fsync, etc
     */
    temp_stream.flush ();
    fsync (fd);
    fbuf.close ();

    /* and now the atomic rename
     */

    if ( rename( ZIX_CONFIG_TEMP_FILENAME, ZIX_CONFIG_FILENAME) )
    {
        lDebug("### Could not save config file; \n");
    }
    else
        parser.get_document()->write_to_file( ZIX_CONFIG_BACKUP_FILENAME );

    return;
}

void
ConfHandler::crypt_passwords ()
{
    std::vector <Glib::ustring> param_ids { "lanFolderPassword",
					    "daDatabasePassword",
					    "lanSocketPassword",
					    "lanWebservicePassword" };

    for (auto id : param_ids) {
	auto pnodes = nodeRoot->find( Glib::ustring::compose ("//parameter[@id='%1']", id ));
	for (auto n : pnodes) {
	    xmlpp::Element *element = dynamic_cast <xmlpp::Element *> (n);
	    if (element) {
		auto value = element->get_attribute_value ("value");
		element->set_attribute ("value", simple_crypt (value, getConfParameter ("macAddress")));
	    }
	}
    }
}

void
ConfHandler::decrypt_passwords ()
{
    std::vector <Glib::ustring> param_ids { "lanFolderPassword",
					    "daDatabasePassword",
					    "lanSocketPassword",
					    "lanWebservicePassword" };

    for (auto id : param_ids) {
	auto pnodes = nodeRoot->find( Glib::ustring::compose ("//parameter[@id='%1']", id ));
	for (auto n : pnodes) {
	    xmlpp::Element *element = dynamic_cast <xmlpp::Element *> (n);
	    if (element) {
		auto value = element->get_attribute_value ("value");
		element->set_attribute ("value", simple_decrypt (value, getConfParameter ("macAddress")));
	    }
	}
    }
}



Glib::ustring ConfHandler::dumpResource(const Glib::ustring& resource)
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Element *element;
    bool added=false;
    Glib::ustring text;

    // Get all parameters for a given resource.
    // The resource "all" is a special resource which iterates through all resources
    nodeSet=nodeRoot->find(
                resource=="all"
                    ? Glib::ustring::compose(
                                "/" ZIX_CONFIG_ROOT_NODE "/%1"
                                , "resource")
                    : Glib::ustring::compose(
                                "/" ZIX_CONFIG_ROOT_NODE "/%1[@id='%2']"
                                , "resource", resource)
                            );

    for (auto&& resource : nodeSet)
    {
        element= dynamic_cast<xmlpp::Element *>( resource );
        if(element)
        {
            text+=element_to_string( element, 0, 1 );
            added=true;
        }
    }

    (void)added;
    return(text);
}

Glib::ustring ConfHandler::getDirectory( const Glib::ustring& id )
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Node::NodeList nodeList;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    Glib::ustring directory;

    nodeSet=nodeRoot->find(
            Glib::ustring::compose(
                            "/" ZIX_CONFIG_ROOT_NODE "/general/directory[@id='%1']"
                            , id ) );
    if(nodeSet.size())
    {
        element= dynamic_cast<xmlpp::Element *>( nodeSet.front() );
        if(!element)
            return directory;

        if( ( attribute=element->get_attribute("value") ) )
            directory=attribute->get_value();
    }

    return( directory );
}

Glib::ustring ConfHandler::convert_share(const Glib::ustring& value) const
{
    Glib::ustring result{value}, back_slash{"\\"}, slash{"/"};

    Glib::ustring::size_type pos = 0;
    while ((pos = value.find(back_slash, pos)) != Glib::ustring::npos) {
        result.replace(pos, back_slash.size(), slash);
        pos += back_slash.size();
    }

    return result;
}

Glib::ustring ConfHandler::getPath(const Glib::ustring& id)
{
    Glib::ustring value = getParameter(id);

    return convert_share(value);
}
Glib::ustring ConfHandler::getParameter(const Glib::ustring& id)
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Node::NodeList nodeList;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    Glib::ustring value;

    nodeSet=nodeRoot->find(
            Glib::ustring::compose(
                            "//parameter[@id='%1']"
                            , id ) );
    if(nodeSet.size())
    {
        element= dynamic_cast<xmlpp::Element *>( nodeSet.front() );
        if(!element)
            return value;

        if( ( attribute=element->get_attribute("value") ) )
            value=attribute->get_value();
    }

    return( value );
}

bool
ConfHandler::getBoolParameter (const Glib::ustring & id)
{
    Glib::ustring str_param = getParameter (id);

    if (str_param == "enabled")
	return true;
    else if (str_param == "Enabled")
	return true;
    else if (str_param == "on")
	return true;
    else if (str_param == "On")
	return true;
    else if (str_param == "true")
	return true;

    /* nothing matched, lets fallback to false
     */
    return false;
}

std::vector<Glib::ustring> ConfHandler::getAllLogFiles()
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Node::NodeList nodeList;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    std::vector<Glib::ustring> files;

    nodeSet=nodeRoot->find("/sic/log/file");

    for (auto&& node : nodeSet) {
        element= dynamic_cast<xmlpp::Element *>( node );
        if(!element)
            continue;

        if( ( attribute=element->get_attribute("path") ) )
            files.emplace_back(attribute->get_value());
    }

    return( files );
}

std::vector<Glib::ustring> ConfHandler::getAllMonitorFiles()
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Node::NodeList nodeList;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    std::vector<Glib::ustring> files;

    nodeSet=nodeRoot->find("/sic/monitor/file");

    for (auto&& node : nodeSet) {
        element= dynamic_cast<xmlpp::Element *>( node );
        if(!element)
            continue;

        if( ( attribute=element->get_attribute("path") ) )
            files.emplace_back(attribute->get_value());
    }

    return( files );
}

std::size_t ConfHandler::getLogBufferSize()
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Node::NodeList nodeList;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    Glib::ustring value;

    nodeSet=nodeRoot->find("/sic/log/logBufferSize");
    if(nodeSet.size())
    {
        element= dynamic_cast<xmlpp::Element *>( nodeSet.front() );
        if(!element)
            return 0;

        if( ( attribute=element->get_attribute("value") ) )
            value = attribute->get_value();
    }

    if (value.empty())
        return 0;

    std::stringstream ss{value};
    std::size_t size;
    ss >> size;

    /* check, if all went well,
     * otherwise size might be invalid, then we return 0 here.
     */
    if (ss.fail() || ss.bad())
	return 0;

    return size;
}

Glib::ustring ConfHandler::getLogLevel()
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Node::NodeList nodeList;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    Glib::ustring value;

    nodeSet=nodeRoot->find("/sic/log/logLevel");

    if(nodeSet.size())
    {
        element= dynamic_cast<xmlpp::Element *>( nodeSet.front() );
        if(!element)
            return value;

        if( ( attribute=element->get_attribute("value") ) )
            value=attribute->get_value();
    }

    return( value );
}

Glib::ustring ConfHandler::getAllocationProcessor(const Glib::ustring& id)
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Node::NodeList nodeList;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    Glib::ustring value;

    nodeSet=nodeRoot->find(
        Glib::ustring::compose( "/" ZIX_CONFIG_ROOT_NODE "/dataAllocation/allocationProcessor[@id='%1']", id));

    if(nodeSet.size())
    {
        element= dynamic_cast<xmlpp::Element *>( nodeSet.front() );
        if(!element)
            return value;

        if( ( attribute=element->get_attribute("code") ) )
            value=attribute->get_value();
    }

    return( value );
}

InputProcessor ConfHandler::getInputProcessorByType(const Glib::ustring& type)
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Node::NodeList nodeList;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    InputProcessor result;

    nodeSet=nodeRoot->find(
        Glib::ustring::compose("/" ZIX_CONFIG_ROOT_NODE "/inputProcessing/inputProcessor[@type='%1']", type));

    if(nodeSet.size())
    {
        element= dynamic_cast<xmlpp::Element *>( nodeSet.front() );
        if(!element)
            return result;

        if( ( attribute=element->get_attribute("id") ) )
            result.id() = attribute->get_value();
        if( ( attribute=element->get_attribute("type") ) )
            result.type() = attribute->get_value();
        if( ( attribute=element->get_attribute("code") ) )
            result.code() = attribute->get_value();
        if( ( attribute=element->get_attribute("pattern") ) )
            result.pattern() = attribute->get_value();
    }

    return( result );
}

std::vector<InputProcessor> ConfHandler::getInputProcessorsByResource(const Glib::ustring& resource)
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    std::vector<InputProcessor> result;

    nodeSet=nodeRoot->find("/sic/inputProcessing/inputProcessor");

    for (auto&& node : nodeSet) {
        InputProcessor ip;
        std::vector<Glib::ustring> res;
        element= dynamic_cast<xmlpp::Element *>( node );
        if(!element)
            continue;

        // here comes the interesting part -> resources can be everywhere
        if( ( attribute=element->get_attribute("resource") ) )
            res.emplace_back(attribute->get_value());

        for (auto&& child : node->get_children()) {
            auto *en = dynamic_cast<xmlpp::Element *>(child);
            if (!en || en->get_name() != "resource")
                continue;
            auto id_attr = en->get_attribute("id");
            if (!id_attr)
                continue;
            res.emplace_back(id_attr->get_value());
        }

        if (std::find(res.begin(), res.end(), resource) == res.end())
            continue;

        ip.resources() = res;
        if( ( attribute=element->get_attribute("id") ) )
            ip.id() = attribute->get_value();
        if( ( attribute=element->get_attribute("type") ) )
            ip.type() = attribute->get_value();
        if( ( attribute=element->get_attribute("code") ) )
            ip.code() = attribute->get_value();
        if( ( attribute=element->get_attribute("pattern") ) )
            ip.pattern() = attribute->get_value();

        result.emplace_back(ip);
    }

    return( result );
}

SerialProtocol ConfHandler::getSerialProtocolHandler(const Glib::ustring& protocol_id)
{
    Glib::ustring id;
    xmlpp::NodeSet nodeSet;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    SerialProtocol result;

    // no protocol -> use default one
    id = protocol_id;
    if (id.empty()) {
        id = getParameter("serialProtocol");
        if (id.empty())
            return result;
    }

    nodeSet=nodeRoot->find(
        Glib::ustring::compose(
            "/sic/interfaces/interface[@id='serial']/resource[@id='serial']/protocol[@id='%1']",
            id));

    if (nodeSet.size()) {
        element = dynamic_cast<xmlpp::Element *>(nodeSet.front());
        if (!element)
            return result;

        if ((attribute = element->get_attribute("id")))
            result.id() = attribute->get_value();
        if ((attribute = element->get_attribute("code")))
            result.code() = attribute->get_value();

        // schemes are not needed atm
    }

    return result;
}

LanProtocol ConfHandler::getLanProtocolHandler(const Glib::ustring& protocol_id)
{
    Glib::ustring id;
    xmlpp::NodeSet nodeSet;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    LanProtocol result;

    // no protocol -> use default one
    id = protocol_id;
    if (id.empty()) {
        id = getParameter("lanSocketProtocol");
        if (id.empty())
            return result;
    }

    nodeSet=nodeRoot->find(
        Glib::ustring::compose(
            "/sic/interfaces/interface[@id='lan']/resource[@id='lanSocket']/protocol[@id='%1']",
            id));

    if (nodeSet.size()) {
        element = dynamic_cast<xmlpp::Element *>(nodeSet.front());
        if (!element)
            return result;

        if ((attribute = element->get_attribute("id")))
            result.id() = attribute->get_value();
        if ((attribute = element->get_attribute("code")))
            result.code() = attribute->get_value();
    }

    return result;
}

Glib::ustring ConfHandler::getValueOrValueRef(xmlpp::Element *elem)
{
    Glib::ustring ret;

    if (!elem)
        return ret;

    auto *val_attr     = elem->get_attribute("value");
    auto *val_ref_attr = elem->get_attribute("valueRef");

    if (val_attr)
        return val_attr->get_value();

    return getParameter(val_ref_attr->get_value());
}

std::vector<Folder> ConfHandler::getFolder(const FileDestination& dest, const Glib::ustring& type)
{
    Glib::ustring id;
    xmlpp::NodeSet nodeSet;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    std::vector<Folder> result;

    nodeSet=nodeRoot->find(
        Glib::ustring::compose(
            "//interface/resource[@id='%1']/output[@type='%2']",
            dest.to_string(), type));

    // hm, try alternative xpath b/o the config isn't really unified
    if (nodeSet.size() == 0)
        nodeSet=nodeRoot->find(
            Glib::ustring::compose(
                "//interface/resource[@id='%1']/output[@id='%2']",
                dest.to_string(), type));

    for (auto&& node : nodeSet) {
        Folder folder;
        element = dynamic_cast<xmlpp::Element *>(node);
        if (!element)
            return result;

        // type/id are attributes
        folder.type() = type;
        if ((attribute = element->get_attribute("id")))
            folder.id() = attribute->get_value();

        // path
        auto *path_child   = element->get_first_child("path");
        auto *format_child = element->get_first_child("format");
        if (path_child)
            folder.path() = convert_share(getValueOrValueRef(dynamic_cast<xmlpp::Element *>(path_child)));
        if (format_child)
            folder.format() = getValueOrValueRef(dynamic_cast<xmlpp::Element *>(format_child));
        PRINT_DEBUG("Adding output: path=" << folder.path() << "; format=" << folder.format());
        result.push_back(folder);
    }

    return result;
}

Timer ConfHandler::getTimerByValueOrValueRef(xmlpp::Element *elem)
{
    Timer t;

    if (!elem)
        return t;

    // given by value?
    auto *value_attr = elem->get_attribute("value");
    if (value_attr) {
        auto *unit_attr = elem->get_attribute("unit");
        if (!unit_attr)
            return t;
        t.value() = value_attr->get_value();
        t.unit()  = unit_attr->get_value();
        return t;
    }

    // timers can also be given by valueRef
    auto *value_ref_attr = elem->get_attribute("valueRef");
    if (!value_ref_attr)
        return t;

    auto param = value_ref_attr->get_value();

    // xpath -> find the parameter
    xmlpp::NodeSet nodeSet;

    nodeSet = nodeRoot->find(
        Glib::ustring::compose("//parameter[@id='%1']", param));
    if (!nodeSet.size())
        return t;

    auto *par_child = dynamic_cast<xmlpp::Element *>(nodeSet.front());
    if (!par_child)
        return t;

    // we have it -> get unit and value
    value_attr      = par_child->get_attribute("value");
    auto *unit_attr = par_child->get_attribute("unit");

    if (!value_attr || !unit_attr)
        return t;

    t.value() = value_attr->get_value();
    t.unit()  = unit_attr->get_value();

    return t;
}

std::vector<Folder> ConfHandler::getInputFolder(const FileDestination& dest, const Glib::ustring& type)
{
    Glib::ustring id;
    xmlpp::NodeSet nodeSet;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    std::vector<Folder> result;

    if (!type.empty()) {
        nodeSet=nodeRoot->find(
            Glib::ustring::compose(
                "//interface/resource[@id='%1']/input[@type='%2']",
                dest.to_string(), type));

        // hm, try alternative xpath b/o the config isn't really unified
        if (nodeSet.size() == 0)
            nodeSet=nodeRoot->find(
                Glib::ustring::compose(
                    "//interface/resource[@id='%1']/input[@id='%2']",
                    dest.to_string(), type));
    } else {
        nodeSet=nodeRoot->find(
            Glib::ustring::compose(
                "//interface/resource[@id='%1']/input",
                dest.to_string()));
    }

    for (auto&& node : nodeSet) {
        Folder folder;
        element = dynamic_cast<xmlpp::Element *>(node);
        if (!element)
            return result;

        // type/id are attributes
        if ((attribute = element->get_attribute("type")))
            folder.type() = attribute->get_value();
        if ((attribute = element->get_attribute("id")))
            folder.id() = attribute->get_value();

        // path
        auto *path_child   = element->get_first_child("path");
        auto *format_child = element->get_first_child("format");
        auto *timer_child  = element->get_first_child("timer");
        if (path_child)
            folder.path() = convert_share(getValueOrValueRef(dynamic_cast<xmlpp::Element *>(path_child)));
        if (format_child)
            folder.format() = getValueOrValueRef(dynamic_cast<xmlpp::Element *>(format_child));

        // input folders can have timers, that's used for e.g. LAN watchdogs
        if (timer_child)
            folder.timer() = getTimerByValueOrValueRef(dynamic_cast<xmlpp::Element *>(timer_child));

        result.push_back(folder);
    }

    return result;
}

std::vector<PostProcessor> ConfHandler::getAllPostProcessors()
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    std::vector<PostProcessor> result;

    nodeSet = nodeRoot->find("/sic/postProcessing/postProcessor");

    for (auto&& node : nodeSet) {
        PostProcessor pp;
        element = dynamic_cast<xmlpp::Element *>(node);
        if (!element)
            continue;

        // id/code/ext are attributes
        if ((attribute = element->get_attribute("id")))
            pp.id() = attribute->get_value();
        if ((attribute = element->get_attribute("code")))
            pp.code() = attribute->get_value();
        if ((attribute = element->get_attribute("ext")))
            pp.ext() = attribute->get_value();

        // formats
        for (auto&& child : element->get_children()) {
            Format format;
            if (child->get_name() != "format")
                continue;

            auto *en = dynamic_cast<xmlpp::Element *>(child);
            if (!en)
                continue;

            // id/ext/tag
            if ((attribute = en->get_attribute("id")))
                format.id() = attribute->get_value();
            if ((attribute = en->get_attribute("ext")))
                format.ext() = attribute->get_value();
            else
                format.ext() = pp.ext();
            if ((attribute = en->get_attribute("tag")))
                format.tag() = attribute->get_value();

            pp.formats().push_back(format);
        }

        result.push_back(pp);
    }

    return result;
}

PostProcessor ConfHandler::getSpecificPostProcessor(
    const FileDestination& dest, const Glib::ustring& format)
{
    xmlpp::NodeSet nodeSet;
    xmlpp::Element* element;
    xmlpp::Attribute* attribute;
    std::vector<Glib::ustring> ids;
    PostProcessor result;

    // get available post processors by resource (ids)
    nodeSet = nodeRoot->find(
        Glib::ustring::compose("//interface/resource[@id='%1']/postProcessor",
                               dest.to_string()));

    for (auto&& node : nodeSet) {
        element = dynamic_cast<xmlpp::Element *>(node);
        if (!element)
            continue;

        // get id
        if ((attribute = element->get_attribute("id")))
            ids.push_back(attribute->get_value());
    }

    // get all pps
    auto all = getAllPostProcessors();
    std::vector<PostProcessor> pps;

    // keep "known" ones
    for (auto&& pp : all) {
        if (std::find_if(ids.begin(), ids.end(),
                         [&pp] (const Glib::ustring& id) -> bool
                         {
                             return id == pp.id();
                         }) != ids.end())
            pps.push_back(pp);
    }

    // try to find right for format
    for (auto&& pp : pps)
        for (auto&& f : pp.formats())
            if (format == f.id())
                return pp;

    return result;
}

PostProcessor ConfHandler::getSpecificPostProcessor(const Glib::ustring& format)
{
    PostProcessor result;

    // get all pps
    auto all = getAllPostProcessors();

    // try to find right for format
    for (auto&& pp : all)
        for (auto&& f : pp.formats())
            if (format == f.id())
                return pp;

    return result;
}

Glib::ustring ConfHandler::getRootFolder(const FileDestination& dest)
{
    if (dest == FileDestination::Dest::usb) {
        auto usb_path = WatchdogManager::get_instance()->usb_mount_path();
        if (usb_path.empty())
            EXCEPTION("Seems like no USB device is mounted.");

        return usb_path;
    }

    if (!SambaMounter::get_instance()->is_mounted()) {
	/* when no instance is mounted, we try to
	 * start a mount process again
	 */
	SambaMounter::get_instance()->mount ();

	/* then we just throw an exception
	 */
        EXCEPTION("Seems like no shared folder is mounted.");
    }

    return SambaMounter::get_instance()->mount_path();
}

std::string ConfHandler::get_config_file_path() const noexcept
{
    return ZIX_CONFIG_FILENAME;
}


Glib::ustring ConfHandler::updatePrinters(const std::list<std::string> printers)
{
    xmlpp::NodeSet nodeSet;
    xmlpp::NodeSet nodeSetPrinters;
    xmlpp::NodeSet nodeSetTemp;
    xmlpp::Element *element, *element2;
    Glib::ustring text;
    xmlpp::Node *startNode=nodeRoot;

    (void)printers;

   nodeSet=startNode->find( ".//interface/resource[@id='lanPrinter']" );
   if(nodeSet.size()==0)
      throw std::logic_error( "updatePrinters: cant find lanPrinter resource" );

   startNode=nodeSet.at(0);

    for (auto&& resource : nodeSet)
    {
        element= dynamic_cast<xmlpp::Element *>( resource );
        if(element)
        {
            nodeSetTemp=element->find( "printer" );

            for (auto&& resource2 : nodeSetTemp)
            {
                element2= dynamic_cast<xmlpp::Element *>( resource2 );
                xmlpp::Attribute *attribute=element2->get_attribute("id");
                lDebug("Removeing printer: %s\n", attribute->get_value().c_str());
                element->remove_child(resource2);
            }
        }
        for(auto &&printer : printers)
        {
            lDebug("Adding printer: %s\n", printer.c_str());
            element2=resource->add_child("printer");
            element2->set_attribute("id", printer);
        }
    }

    save();

    return(text);
}


//---fin.----------------------------------------------------------------------
