//----------------------------------------------------------------------------
///
/// \file   udb_channel.cc
///
/// \brief  Multicast UDP Sender/Receiver class
///
///         send and receive stuff
///
/// \date   [20161209] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//----------------------------------------------------------------------------


//---Includes-----------------------------------------------------------------


//---General-------------------------

#include <giomm.h>
#include <iostream>

//---Own-----------------------------

#include "udp_channel.h"


//---Implementation-----------------------------------------------------------


#define     BROADCAST_PORT      7777
#define     BROADCAST_INTERFACE "eth0"


static bool use_ipv6 = false;


Glib::ustring
socket_address_to_string(const Glib::RefPtr<Gio::SocketAddress>& address)
{
  Glib::RefPtr<Gio::InetAddress> inet_address;
  Glib::ustring str, res;
  int port;

  auto isockaddr = Glib::RefPtr<Gio::InetSocketAddress>::cast_dynamic(address);
  if (!isockaddr)
    return Glib::ustring();
  inet_address = isockaddr->get_address();
  str = inet_address->to_string();
  port = isockaddr->get_port();
  res = Glib::ustring::compose("%1:%2", str, port);
  return res;
}


/// \brief  Le contructeur
CUdpChannel::CUdpChannel()
{
    address_dummy=Gio::InetSocketAddress::create(Gio::InetAddress::create("0.0.0.0"), BROADCAST_PORT);

    socket_type = Gio::SOCKET_TYPE_DATAGRAM; //  : Gio::SOCKET_TYPE_STREAM
    socket_family = use_ipv6 ? Gio::SOCKET_FAMILY_IPV6 : Gio::SOCKET_FAMILY_IPV4;

    try
    {
        socket = Gio::Socket::create(socket_family, socket_type, Gio::SOCKET_PROTOCOL_UDP);
    }
    catch (const Gio::Error& error)
    {
        std::cerr << Glib::ustring::compose("%1\n", error.what());
        return;
    }

    try
    {
        // Receiver / Listener / Server
        socket->bind( address_dummy, true /*allow_reuse*/);

        // Sender / Client;
        // Must NOT connect or no data will be send for some reason
        // socket->connect(address, cancellable);

        socket->set_broadcast(true);
        socket->set_blocking(false);
        //socket->set_multicast_loopback( false );
    }
    catch (const Gio::Error& error)
    {
      std::cerr << Glib::ustring::compose("Connection to %1 failed: %2, trying next\n",
        socket_address_to_string(address), error.what());
    }

  std::cout << Glib::ustring::compose("Connected to %1\n", socket_address_to_string(address));
}


CUdpChannel::~CUdpChannel()
{
  std::cout << "closing socket\n";

  try
  {
    socket->close();
  }
  catch (const Gio::Error& error)
  {
    std::cerr << Glib::ustring::compose("Error closing master socket: %1\n", error.what());
  }

}


int CUdpChannel::send(const Glib::RefPtr<Gio::SocketAddress>& address,
                      const char *buffer, int size)
{
    gsize to_send;

    to_send=size;

    while (to_send > 0)
    {
      //ensure_condition(socket, "send", cancellable, Glib::IO_OUT);
      try
      {
          size = socket->send_to(address, buffer, to_send, cancellable);
      }
      catch (const Gio::Error& error)
      {
        if (error.code() == Gio::Error::WOULD_BLOCK)
        {
          std::cout << "socket send would block, handling\n";
          continue;
        }
        else
        {
          std::cerr << Glib::ustring::compose("Error sending to socket: %1 (%2)\n", error.what(), size);
          return 1;
        }
      }

      std::cout << Glib::ustring::compose("sent %1 bytes of data\n", size);

      if (size == 0)
      {
        std::cerr << "Unexpected short write\n";
        return 1;
      }

      to_send -= size;
    }

    std::cout << "Sending done\n";

    return(1);
}


int CUdpChannel::receive(Glib::RefPtr<Gio::SocketAddress>& address, char *buffer, int max_size)
{
    gssize size;

    try
    {
        size = socket->receive_from(address, buffer, max_size);
    }
    catch (const Gio::Error& error)
    {
      std::cerr << Glib::ustring::compose("Error receiving from socket: %1\n", error.what());
      return 0;
    }

    if (size == 0)
      return(0);

    /*
    std::cout << Glib::ustring::compose("received %1 bytes of data from %1\n", size
                        , socket_address_to_string(address));

    dump(buffer, size);
    */

    return (size);
}


//---fin.---------------------------------------------------------------------
