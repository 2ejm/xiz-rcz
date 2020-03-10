#ifndef _UNIX_SOCKET_INTERFACE_HANDLER_H_
#define _UNIX_SOCKET_INTERFACE_HANDLER_H_

#include "interface_handler.h"
#include "gio_istream_adapter.h"
#include "xml_processor.h"
#include "zix_interface.h"

#include <giomm/socketservice.h>
#include <giomm/unixsocketaddress.h>
#include <glibmm/ustring.h>

#include <string>
#include <memory>

/**
 * \brief UNIX Socket Interface
 */
class UnixSocketInterfaceHandler : public InterfaceHandler
{
public:
    using RefPtr = Glib::RefPtr<UnixSocketInterfaceHandler>;

    UnixSocketInterfaceHandler(ZixInterface inf, const std::string& path, Glib::RefPtr <XmlProcessor> xml_processor);

    static RefPtr create(ZixInterface inf, const std::string& path,
                         Glib::RefPtr <XmlProcessor> xml_processor);

    virtual ~UnixSocketInterfaceHandler();

private:
    Glib::RefPtr<Gio::SocketService> _service;
    Glib::RefPtr<Gio::UnixSocketAddress> _unix_addr;

    bool incoming_connection(const Glib::RefPtr<Gio::SocketConnection>& connection,
                             const Glib::RefPtr<Glib::Object>& source_object);

    void closed_connection(std::weak_ptr<InterfaceConnection> conn);
};

#endif /* _UNIX_SOCKET_INTERFACE_HANDLER_H_ */
