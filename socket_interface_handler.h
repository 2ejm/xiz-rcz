
#ifndef LIBZIX_SOCKET_INTERFACE_HANDLER
#define LIBZIX_SOCKET_INTERFACE_HANDLER

#include "interface_handler.h"
#include "gio_istream_adapter.h"
#include "xml_processor.h"
#include "zix_interface.h"

#include <giomm/inetsocketaddress.h>
#include <giomm/inetaddress.h>
#include <giomm/socketservice.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

#include <memory>

/**
 * \brief TCP Socket Interface
 *
 * Opens TCP Socket, on each Connection
 * Read until EOF, then emit istream_ready signal
 *
 * \note unsure about whether we can terminate the
 *       InputStream via EOF, and keep the OutputStream
 *       open for submitting the reply.
 *
 *       Maybe we need a terminator.
 */
class SocketInterfaceHandler : public InterfaceHandler
{
public:
    using RefPtr = Glib::RefPtr <SocketInterfaceHandler>;

    SocketInterfaceHandler(ZixInterface inf, int port, Glib::RefPtr <XmlProcessor> xml_processor);

    static RefPtr create(ZixInterface inf, int port, Glib::RefPtr <XmlProcessor> xml_processor);

    virtual ~SocketInterfaceHandler();

private:
    Glib::RefPtr<Gio::SocketService> _service;
    Glib::RefPtr<Gio::InetSocketAddress> _socket_addr;
    Glib::RefPtr<Gio::InetAddress> _inet_addr;

    bool incoming_connection (const Glib::RefPtr<Gio::SocketConnection>& connection,
                              const Glib::RefPtr<Glib::Object>& source_object);
    void closed_connection(std::weak_ptr<InterfaceConnection> conn);
};

#endif
