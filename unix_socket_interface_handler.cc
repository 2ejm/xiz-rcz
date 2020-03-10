#include <giomm/socketaddress.h>

#include <sigc++/signal.h>

#include "unix_socket_interface_handler.h"
#include "socket_interface_connection.h"
#include "byteslist_istream.h"
#include "file_handler.h"
#include "hexio.h"
#include "utils.h"

UnixSocketInterfaceHandler::UnixSocketInterfaceHandler(
    ZixInterface inf, const std::string& path, Glib::RefPtr <XmlProcessor> xml_processor) :
    Glib::ObjectBase(typeid (UnixSocketInterfaceHandler)),
    InterfaceHandler(inf, inf.to_string(), xml_processor)
{
    // if socket exists (e.g. unclean shutdown of our daemon) -> delete it
    // otherwise, the creation code here will throw an exception
    try {
        if (FileHandler::socket_exists(path))
            FileHandler::del_file(path);
    } catch (...) {
    }

    try {
        Glib::RefPtr<Gio::SocketAddress> effective_addr;
        _service   = Gio::SocketService::create();
        _unix_addr = Gio::UnixSocketAddress::create(path,
                                                    Gio::UNIX_SOCKET_ADDRESS_PATH, -1);
        _service->add_address(_unix_addr, Gio::SOCKET_TYPE_STREAM,
                              Gio::SOCKET_PROTOCOL_DEFAULT, effective_addr);

#ifdef GLOBAL_INSTALLATION
        // setup permissions: www-data should have access to the unix sockets
        FileHandler::chown(path, "root", "www-data");
        FileHandler::chmod(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
#endif
    } catch (const Glib::Error& ex) {
        EXCEPTION(ex.what());
    } catch (const std::exception& ex) {
        EXCEPTION(ex.what());
    }

    _service->signal_incoming().connect (sigc::mem_fun (*this, &UnixSocketInterfaceHandler::incoming_connection));
}

UnixSocketInterfaceHandler::RefPtr
UnixSocketInterfaceHandler::create(ZixInterface inf, const std::string& path, Glib::RefPtr <XmlProcessor> xml_processor)
{
    return RefPtr(new UnixSocketInterfaceHandler(inf, path, xml_processor));
}

bool UnixSocketInterfaceHandler::incoming_connection (
    const Glib::RefPtr<Gio::SocketConnection>& connection,
    const Glib::RefPtr<Glib::Object>& source_object)
{
    (void) source_object;

    auto iface_connection = SocketInterfaceConnection::create (connection);
    register_connection (iface_connection);

    iface_connection->connection_closed.connect(
        sigc::bind<std::weak_ptr<InterfaceConnection> >(
            sigc::mem_fun(*this, &UnixSocketInterfaceHandler::closed_connection),
            std::weak_ptr<InterfaceConnection>(iface_connection)));

    return true;
}

void UnixSocketInterfaceHandler::closed_connection(std::weak_ptr<InterfaceConnection> conn)
{
    deregister_connection(conn);
}

UnixSocketInterfaceHandler::~UnixSocketInterfaceHandler()
{
}
