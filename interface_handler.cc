#include "interface_handler.h"
#include "interface_connection.h"
#include "utils.h"

#include "xml_processor.h"

std::map<Glib::ustring, Glib::RefPtr<InterfaceHandler>> InterfaceHandler::interface_map;

InterfaceHandler::InterfaceHandler (ZixInterface inf, const Glib::ustring &name, Glib::RefPtr <XmlProcessor> xml_processor) :
    StartStoppable(inf),
    _inf (inf), _name (name), _xml_processor (xml_processor)
{ }

InterfaceHandler::~InterfaceHandler()
{
}

void
InterfaceHandler::register_connection (std::shared_ptr <InterfaceConnection> conn)
{
    PRINT_DEBUG("New incomming connection");
    _connection_list.push_back (conn);

    // Incomming requests are always send to the xml_processor.
    conn->request_ready.connect (sigc::bind (sigc::mem_fun (_xml_processor.operator->(), &XmlProcessor::parse_stream),
                                             0,
                                             std::weak_ptr<InterfaceConnection>(conn),
                                             _name,
					     false));
}

void
InterfaceHandler::deregister_connection(std::weak_ptr<InterfaceConnection> conn)
{
    PRINT_DEBUG("Deregister connection");
    auto refptr = conn.lock();
    _connection_list.remove(refptr);
}

void
InterfaceHandler::register_handler (Glib::RefPtr <InterfaceHandler> handler)
{
    interface_map[handler->_name] = handler;
}

Glib::RefPtr<InterfaceHandler> InterfaceHandler::getInterface( const Glib::ustring &name ) /* static */
{
    return(interface_map[name]);
}


//---fin---------------------------------------------------------------------ä
