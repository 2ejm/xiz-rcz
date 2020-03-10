
#include "socket_interface_handler.h"
#include "socket_interface_connection.h"
#include "byteslist_istream.h"
#include "hexio.h"
#include "utils.h"

SocketInterfaceHandler::SocketInterfaceHandler(
    ZixInterface inf, int port, Glib::RefPtr <XmlProcessor> xml_processor)
    : Glib::ObjectBase (typeid (SocketInterfaceHandler))
    , InterfaceHandler (inf, inf.to_string(), xml_processor)
{
    try {
        Glib::RefPtr<Gio::SocketAddress> effective_addr;

        _service     = Gio::SocketService::create();
#ifdef SOCKET_OPEN
        _inet_addr   = Gio::InetAddress::create_any(Gio::SOCKET_FAMILY_IPV4);
#else
        _inet_addr   = Gio::InetAddress::create_loopback(Gio::SOCKET_FAMILY_IPV4);
#endif
        _socket_addr = Gio::InetSocketAddress::create(_inet_addr, port);

        _service->add_address(_socket_addr, Gio::SOCKET_TYPE_STREAM,
                              Gio::SOCKET_PROTOCOL_DEFAULT, effective_addr);
    } catch (const Glib::Error& ex) {
        EXCEPTION(ex.what());
    }

    _service->signal_incoming().connect (sigc::mem_fun (*this, &SocketInterfaceHandler::incoming_connection));
}

SocketInterfaceHandler::RefPtr
SocketInterfaceHandler::create(ZixInterface inf, int port, Glib::RefPtr <XmlProcessor> xml_processor)
{
    return RefPtr(new SocketInterfaceHandler (inf, port, xml_processor));
}

bool
SocketInterfaceHandler::incoming_connection (
	const Glib::RefPtr<Gio::SocketConnection>& connection,
	const Glib::RefPtr<Glib::Object>& source_object)
{
    (void) source_object;

    auto iface_connection = SocketInterfaceConnection::create (connection);
    register_connection (iface_connection);

    iface_connection->connection_closed.connect(
        sigc::bind<std::weak_ptr<InterfaceConnection> >(
            sigc::mem_fun(*this, &SocketInterfaceHandler::closed_connection),
            std::weak_ptr<InterfaceConnection>(iface_connection)));

    return true;
}

void SocketInterfaceHandler::closed_connection(std::weak_ptr<InterfaceConnection> conn)
{
    deregister_connection(conn);
}

SocketInterfaceHandler::~SocketInterfaceHandler()
{
}

