#ifndef _VISUX_H
#define _VISUX_H
//-----------------------------------------------------------------------------
///
/// \brief  VisuX protocol handler
///
///         The mechanism is a trivial request-response ping-pong
///
/// \date   [20161209] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

//#include <glibmm/ustring.h>
//#include <glibmm/refptr.h>
//#include <glibmm/object.h>
#include <glibmm/keyfile.h>
#include <string.h>             // memcpy


//---Own------------------------------

#include "udp_channel.h"
#include "socket_client.h"
#include "network_helpers.h"     // types
#include "conf_handler.h"
#include "query_client.h"


//---Declaration---------------------------------------------------------------


#define SEARCH_COMMAND_SIZE         8
#define SEARCH_RESPONSE_SIZE        91
#define ASSIGN_COMMAND_SIZE         42
#define ASSIGN_RESPONSE_SIZE        16
#define RESET_COMMAND_SIZE          16
#define RESET_RESPONSE_SIZE         16

#if ! defined ( VISUX_INTERFACE )
    #define VISUX_INTERFACE "eth0"
#endif


enum EVisuxMode
{
    eVisuxModeDaemon,
    eVisuxModeController,
};

typedef struct __attribute__ ( ( packed ) )
{
    char header[8];
    mac_t mac;
    char flags;
    ip_t ip;
    ip_t subnet;
    ip_t dns1;
    ip_t dns2;
    ip_t ntp_server;
    char ws_port[5];
    char device_name[20];
    char device_serial[10];
    char firmware_version_dc[10];
    char firmware_version_sic[10];
} SSearchResponse;


typedef struct __attribute__ ( ( packed ) )
{
    char header[8];
    mac_t mac;
    char control_flags;
    char network_flags;
    ip_t ip;
    ip_t subnet;
    ip_t dns1;
    ip_t dns2;
    ip_t ntp_server;
    char ws_port[5];
} SAssignCommand;

typedef struct __attribute__ ( ( packed ) )
{
    char header[8];
    mac_t mac;
    char control_flags;
} SResetCommand;

enum ENetworkFlags
{
    eNetworkFlagsStatic = 0,
    eNetworkFlagsDhcp = 1,
};

class CNetworkConfig
            :public CUdpChannel
{
    private:
	static Glib::RefPtr <CNetworkConfig> instance;

        //CUdpChannel udpChannel;
        //CReceiver receiver;
        char *buffer;
        int maxBufferSize;
        EVisuxMode visuxMode;

        // Data
        char controlFlags;
        char networkFlags;
        mac_t mac;
        ip_t ip;
        ip_t subnet;
        ip_t dns1, dns2;
        ip_t gateway;
        ip_t ntp_server;
        port_t webservice_port;

        Glib::RefPtr <ConfHandler> confHandler;

	bool reread_network_config();

    protected:
        bool slotReceiveFunc( Glib::IOCondition condition );

    public:
        CNetworkConfig( EVisuxMode _visuxMode );
        ~CNetworkConfig( );
	static Glib::RefPtr <CNetworkConfig> get_instance ();

        static Glib::RefPtr <CNetworkConfig>create(EVisuxMode _visuxMode);

    Glib::RefPtr <Query> _state_query;

        void collectData();
	void collectNtpData ();
        void checkBackNetworkConfig();
        void info(void);

        int parseBuffer(const Glib::RefPtr<Gio::SocketAddress>& addr, int size);

        void setWebservicePort(int port)
        {
            webservice_port=port;
        }

        void setMac(const mac_t &_mac)
        {
            //00:26:2D:F5:56:2D
            memcpy( mac, _mac, sizeof(mac) );
        };

        void erase();
        void reset();
        void waste();
        void fillStaticIp();
        void writeToConfig( bool ignoreChanges = false );
        void write_to_config_net_only( bool ignoreChanges = false );

        void handleSearchCommand(const Glib::RefPtr<Gio::SocketAddress>& addr, int size );
        void handleAssignCommand(const Glib::RefPtr<Gio::SocketAddress>& addr, int size );
        void handleResetCommand(const Glib::RefPtr<Gio::SocketAddress>& addr, int size );

        void digestSearchResponse( int size );
        void digestAssignResponse( int size );
        void digestResetResponse( int size );

        int runCommand( const Glib::ustring &command);
        int updateNetwork();
        int updateNtpConfig();
        int updateNetworkConfig();

        // Listen to the config handler for network changes
        void slot_conf_change_announce(const Glib::ustring& par_id, const Glib::ustring& value, int &handlerMask);
        void slot_conf_changed(int handlerMask);

	bool online ();

    void replay_network_settings(bool mode_only = false);
    void on_set_mode_parameter_finished(const Glib::RefPtr<XmlResult>& result, bool mode_only);
    void on_set_lan_parameters_finished(const Glib::RefPtr<XmlResult>& result);
};


//-----------------------------------------------------------------------------
#endif // ? ! _VISUX_H
