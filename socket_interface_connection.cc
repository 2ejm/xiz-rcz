
#include "socket_interface_connection.h"
#include "procedure_step_handler.h"

#include "byteslist_istream.h"

SocketInterfaceConnection::SocketInterfaceConnection (const Glib::RefPtr <Gio::SocketConnection> & connection)
    : InterfaceConnection (ZIX_PROC_RESULT_FILE)
    , _connection (connection)
{
    auto input_stream = connection->get_input_stream();

    _adapter = GioIstreamAdapter::create (input_stream);
    _adapter->stream_read.connect (sigc::mem_fun (*this, &SocketInterfaceConnection::stream_complete));
    _adapter->stream_eof.connect (sigc::mem_fun (*this, &SocketInterfaceConnection::stream_eof));
}

SocketInterfaceConnection::~SocketInterfaceConnection ()
{ }

void
SocketInterfaceConnection::emit_result (const Glib::ustring & result, int tid)
{
    (void) tid;

    auto output_stream = _connection->get_output_stream ();

    char c = '\0';
    Glib::ustring::size_type written = 0;
    while (written < result.length())
        written += output_stream->write (result.substr(written));
    output_stream->write (&c, 1);
    output_stream->flush ();
}

void
SocketInterfaceConnection::cancel ()
{
}


std::shared_ptr <InterfaceConnection>
SocketInterfaceConnection::create (const Glib::RefPtr <Gio::SocketConnection> & connection)
{
    return std::shared_ptr <InterfaceConnection> (new SocketInterfaceConnection (connection));
}

void SocketInterfaceConnection::stream_eof()
{
    connection_closed.emit();
}

void
SocketInterfaceConnection::stream_complete (std::list <Glib::RefPtr <Glib::Bytes> > bytes_list)
{
    if (bytes_list.size () == 0)
	return;

    BytesListIStream is_buf (bytes_list);
    std::istream is (&is_buf);

    request_ready.emit (is, 0);
}

