#ifndef _SENDER_H
#define _SENDER_H
//-----------------------------------------------------------------------------
///
/// \brief  Multicast UDP Sender/Receiver class
///
///         send and receive stuff
///
/// \date   [20161209] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

//#include <glibmm/ustring.h>
//#include <glibmm/refptr.h>
#include <glibmm/object.h>
#include <giomm/socket.h>


//---Own------------------------------


//---Declaration---------------------------------------------------------------


class CUdpChannel: public Glib::Object
{
    private:
        //Glib::RefPtr<Gio::SocketAddress> src_address;
        Glib::RefPtr<Gio::SocketAddress> address;       // snd
        Glib::RefPtr<Gio::SocketAddress> address_dummy; // bind
        Gio::SocketType socket_type;
        Gio::SocketFamily socket_family;
        Glib::RefPtr<Gio::Cancellable> cancellable;
        //Glib::RefPtr<Gio::SocketAddressEnumerator> enumerator;
        //Glib::RefPtr<Gio::SocketConnectable> connectable;

    public:
        Glib::RefPtr<Gio::Socket> socket;

        CUdpChannel( );
        ~CUdpChannel( );

        int receive(Glib::RefPtr<Gio::SocketAddress>& address,
                    char *buffer, int max_size);
        int send(const Glib::RefPtr<Gio::SocketAddress>& address,
                 const char *buffer, int size);
};


//-----------------------------------------------------------------------------
#endif // ? ! _SENDER_H
