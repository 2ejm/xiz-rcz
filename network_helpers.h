#ifndef _NETWORK_HELPERS_H
#define _NETWORK_HELPERS_H
//-----------------------------------------------------------------------------
///
/// \brief  networking helper functions
///
///         mac, ip, dns, etc.
///
/// \date   [20161212] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

//#include <glibmm/ustring.h>
//#include <glibmm/refptr.h>
//#include <glibmm/object.h>

//---Own------------------------------

//#include "udp_channel.h"


//---Declaration---------------------------------------------------------------

namespace Glib
{
    class ustring;
}

typedef unsigned char ip_t[4];
typedef unsigned char mac_t[0x6];
typedef unsigned int port_t;

int getMacAddress(const char *interfaceName, mac_t *mac);
int getNetworkAddress(const char *interfaceName, ip_t *ip, ip_t *submask);
int getIpFromString(ip_t *ip, const Glib::ustring &str);
int getPortFromString(const Glib::ustring &str);
int getDefaultGateway ( ip_t *gw );
Glib::ustring getStringFromIp(ip_t *ip);
Glib::ustring getStringFromMac(mac_t *mac);
int getDnsServer( ip_t *dns1, ip_t *dns2);
void dumpMac(mac_t *mac);
void dumpIp(ip_t *ip);
bool isEmptyIp(ip_t *ip);
void setIp(ip_t *ip, int a, int b , int c, int d);


//-----------------------------------------------------------------------------
#endif // ? ! _NETWORK_HELPERS_H
