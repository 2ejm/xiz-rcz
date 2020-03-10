//----------------------------------------------------------------------------
///
/// \file   network_helpers.cc
///
/// \brief  Network related helper functions for visux
///
///         nothing concrete related to visux but used by it.
///
//----------------------------------------------------------------------------


//---Includes-----------------------------------------------------------------


//---General-------------------------

#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sstream>          // stringstream
#include <iomanip>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>             //res_init
#include <glibmm/ustring.h>

//---Own-----------------------------

#include "network_helpers.h"


//---Implementation-----------------------------------------------------------


int getMacAddress( const char *interfaceName, mac_t *mac)
{
    int fd;
    struct ifreq ifr;
    int sta;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, interfaceName, IFNAMSIZ-1);
    sta=ioctl(fd, SIOCGIFHWADDR, &ifr);
    close(fd);

    if(sta)
        memset(mac, 0, sizeof(*mac));
    else
        memcpy(mac, ifr.ifr_hwaddr.sa_data, sizeof(*mac));

    return(sta);
}


/// \brief  returns ip address of specific interface
///
///         IPV4
int getNetworkAddress( const char *interfaceName, ip_t *ip, ip_t *submask )
{
    struct ifreq ifr;
    int fd;
    int sta;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, interfaceName, IFNAMSIZ-1);
    sta=ioctl(fd, SIOCGIFADDR, &ifr);
    if(sta)
        memset(ip, 0, sizeof(*ip));
    else
        memcpy(ip, &ifr.ifr_addr.sa_data[2], sizeof(*ip));
    sta=ioctl(fd, SIOCGIFNETMASK, &ifr);
    if(sta)
        memset(submask, 0, sizeof(*submask));
    else
        memcpy(submask, &ifr.ifr_addr.sa_data[2], sizeof(*submask));

    close(fd);

    return sta;
}


/// \brief  returns dns ip addresses
///
///         IPV4
int getDnsServer( ip_t *dns1, ip_t *dns2)
{
    // Always reinitialize res; config may have changed and we dont current
    // information
    res_init();

    struct __res_state *res_state;

    res_state=__res_state();

    memcpy(dns1, &res_state->nsaddr_list[0].sin_addr.s_addr, sizeof(*dns1));
    memcpy(dns2, &res_state->nsaddr_list[1].sin_addr.s_addr, sizeof(*dns2));

    // memset is neccessarry to clear dns2 if it was set and will be empty
    // afterwards
    memset((void*)res_state, 0, sizeof(*res_state));
    res_close();

    return(0);
}


/// \brief  Converts string with ip to ip
int getIpFromString(ip_t *ip, const Glib::ustring &str)
{
    unsigned char *ptr=(unsigned char *)ip;
    int i1 = 0;
    char ch;
    std::stringstream strstr(str);
    strstr >> i1 >> ch; ptr[0]=i1;
    strstr >> i1 >> ch; ptr[1]=i1;
    strstr >> i1 >> ch; ptr[2]=i1;
    strstr >> i1 >> ch; ptr[3]=i1;

    // printf("Origin String:`%s` -> %d.%d.%d.%d\n", str.c_str(), ptr[0], ptr[1], ptr[2], ptr[3]);
    return(0);
}


/// \brief  Converts string with ip to ip
int getPortFromString(const Glib::ustring &str)
{
    int i1 = 0;
    std::stringstream strstr(str);
    strstr >> i1;
    return(i1);
}


/// \brief  Converts ip into string
Glib::ustring getStringFromIp(ip_t *ip)
{
    unsigned char *ptr=(unsigned char *)ip;
    return( Glib::ustring::compose ("%1.%2.%3.%4", ptr[0], ptr[1], ptr[2], ptr[3]) );
}

Glib::ustring getStringFromMac(mac_t *mac)
{
    unsigned char *ptr=(unsigned char *)mac;

    return Glib::ustring::compose ("%1-%2-%3-%4-%5-%6",
		Glib::ustring::format (std::hex, std::setfill (L'0'), std::setw (2), ptr[0]),
		Glib::ustring::format (std::hex, std::setfill (L'0'), std::setw (2), ptr[1]),
		Glib::ustring::format (std::hex, std::setfill (L'0'), std::setw (2), ptr[2]),
		Glib::ustring::format (std::hex, std::setfill (L'0'), std::setw (2), ptr[3]),
		Glib::ustring::format (std::hex, std::setfill (L'0'), std::setw (2), ptr[4]),
		Glib::ustring::format (std::hex, std::setfill (L'0'), std::setw (2), ptr[5]));
}

/// \brief  Print a mac address on terminal
void dumpMac(mac_t *mac)
{
    unsigned char *ptr=(unsigned char *)mac;
    printf("%02X:%02X:%02X:%02X:%02X:%02X"
           , ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]
           );

}


/// \brief  Print a ip address on terminal
void dumpIp(ip_t *ip)
{
    unsigned char *ptr=(unsigned char *)ip;
    printf("%u.%u.%u.%u"
            , (unsigned int)ptr[0]
            , (unsigned int)ptr[1]
            , (unsigned int)ptr[2]
            , (unsigned int)ptr[3]
            );
    return;
}


void setIp(ip_t *ip, int a, int b , int c, int d)
{
    unsigned char *ptr=(unsigned char *)ip;
    ptr[0]=a;
    ptr[1]=b;
    ptr[2]=c;
    ptr[3]=d;

    return;
}


int getDefaultGateway ( ip_t *gw )
{
    FILE *f;
    char line[100] , *p , *c, *g, *saveptr;
    int nRet=1;

    f = fopen("/proc/net/route" , "r");

    while(fgets(line , 100 , f))
    {
        p = strtok_r(line , " \t", &saveptr);
        c = strtok_r(NULL , " \t", &saveptr);
        g = strtok_r(NULL , " \t", &saveptr);

        if(p!=NULL && c!=NULL)
        {
            if(strcmp(c , "00000000") == 0)
            {
                //printf("Default interface is : %s \n" , p);
                if (g)
                {
                    char *pEnd;
                    int ng=strtol(g,&pEnd,16);

                    memcpy(gw, &ng, sizeof(*gw));
                    nRet=0;
                }
                break;
            }
        }
    }

    fclose(f);
    return nRet;
}


bool isEmptyIp(ip_t *ip)
{
    return( ! ( ip[0] || ip[1] || ip[2] || ip[0] ) );
}

//---fin.---------------------------------------------------------------------
