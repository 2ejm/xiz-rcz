
#ifndef LIBZIX_SOCKET_INTERFACE_CONNECTION_H
#define LIBZIX_SOCKET_INTERFACE_CONNECTION_H

#include "glibmm/refptr.h"
#include <giomm/socketservice.h>
#include <glibmm/ustring.h>

#include <memory>

#include "interface_connection.h"
#include "gio_istream_adapter.h"

class SocketInterfaceConnection : public InterfaceConnection
{
    public:
	SocketInterfaceConnection (const Glib::RefPtr <Gio::SocketConnection> & connection);
	~SocketInterfaceConnection ();

	static std::shared_ptr <InterfaceConnection> create (const Glib::RefPtr <Gio::SocketConnection> & connection);

	void emit_result (const Glib::ustring & result, int tid);
	void cancel ();

    private:
	Glib::RefPtr <Gio::SocketConnection> _connection;
	Glib::RefPtr <GioIstreamAdapter> _adapter;

	void stream_complete (std::list <Glib::RefPtr <Glib::Bytes> > bytes_list);
    void stream_eof();
};
#endif
