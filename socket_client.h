#ifndef LIBZIX_SOCKET_CLIENT
#define LIBZIX_SOCKET_CLIENT
//-----------------------------------------------------------------------------
///
/// \brief  Socket client
///
///         Provides client side for libzix commands
///         See implementation for further details.
///
/// \date   [20161219] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <giomm/socketclient.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

//---Own------------------------------

#include "xml_processor.h"
#include "xml_result_parsed.h"
#include "gio_istream_adapter.h"


//---Implementation------------------------------------------------------------


/**
 * \brief TCP Socket Client
 *
 * Connects TCP Socket
 */
class SocketClient : public Glib::Object
                     //   public InterfaceHandler
{
    public:
    SocketClient ( int port );

    static Glib::RefPtr <SocketClient> create ( int port );

    virtual ~SocketClient();

    int sendRequest(const Glib::ustring &request, Glib::RefPtr<XmlResult> &result);
    int sendRequest(const Glib::ustring &request, Glib::ustring &response);
    void incoming_data ( Gio::SocketClientEvent,
                               const Glib::RefPtr<Gio::SocketConnectable>&,const Glib::RefPtr<Gio::IOStream>& );
    void stream_complete (std::list <Glib::RefPtr <Glib::Bytes> > bytes_list);

    int getConf(const Glib::ustring &item
                        , const Glib::ustring &resource
                        , const Glib::ustring &node
                        , const Glib::ustring &postprocessor
                        , Glib::ustring &response);

    int getConfNetwork( bool &dhcp
                              , Glib::ustring &ip
                              , Glib::ustring &subnetMask
                              , Glib::ustring &dns1
                              , Glib::ustring &dns2
                              , Glib::ustring &gateway
                              , Glib::ustring &ntp );

    int setFile(  const Glib::ustring &host
                , const Glib::ustring &filename
                , const Glib::ustring &data);
    private:
    Glib::RefPtr<Gio::SocketClient> _client;
    Glib::RefPtr<Gio::SocketConnection> _connection;
    Glib::RefPtr <GioIstreamAdapter> _adapter;
    //std::shared_ptr <InterfaceConnection> iface_connection;


};


//---fin.---------------------------------------------------------------------
#endif // ? ! LIBZIX_SOCKET_CLIENT
