//-----------------------------------------------------------------------------
///
/// \brief  Visux protocoll handler
///
///         The mechanism is a trivial request-response ping-pong
///
///         Values:
///             IP: If dhcp is set the real ip is send on search command
///                 but is never stored.
///
/// \date   [20161209] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <giomm/socketsource.h>     // Gio::signal_socket()
#include <glibmm/bytearray.h>
#include <memory.h>                 // memcmp
#include <cassert>
#include <glibmm/spawn.h>
#include <glibmm/regex.h>           // split

//---Own------------------------------

#include "network_config.h"
#include "network_helpers.h"
#include "network_config_helpers.h"
#include "libzix.h"
#include "conf_handler.h"
#include "utils.h"
#include "xml_query_replay_status.h"


//---Implementation------------------------------------------------------------


static bool verbose_network=false;

Glib::RefPtr <CNetworkConfig> CNetworkConfig::instance;

/// \brief  le constructeur
CNetworkConfig::CNetworkConfig( EVisuxMode _visuxMode )
    :visuxMode(_visuxMode)
{
    //sigc::slot<bool, Glib::IOCondition> slot = sigc::ptr_fun(&mySlot);
    sigc::slot<bool, Glib::IOCondition> slot = sigc::mem_fun( (*this), &CNetworkConfig::slotReceiveFunc);

    maxBufferSize=0x100;
    buffer=(char *)malloc(maxBufferSize);

    controlFlags=0;
    networkFlags=0;

    Gio::signal_socket().connect(
                    slot, socket, Glib::IOCondition::IO_IN);

    confHandler=ConfHandler::get_instance();
    collectData();
    collectNtpData ();
    updateNetwork();
    updateNtpConfig ();

    if(verbose_network)
        info();

    auto conf_handler = ConfHandler::get_instance();
    conf_handler->confChangeAnnounce.connect(
        sigc::mem_fun(*this, &CNetworkConfig::slot_conf_change_announce ));
    conf_handler->confChanged.connect(
        sigc::mem_fun(*this, &CNetworkConfig::slot_conf_changed ));
    Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &CNetworkConfig::reread_network_config), 10*1000);
};

Glib::RefPtr <CNetworkConfig> CNetworkConfig::get_instance() /* static */
{
    if (!instance)
	instance = CNetworkConfig::create (eVisuxModeDaemon);

    return instance;
}

Glib::RefPtr <CNetworkConfig>CNetworkConfig::create( EVisuxMode _visuxMode ) /* static */
{
    return ( Glib::RefPtr <CNetworkConfig>( new CNetworkConfig(_visuxMode) ) );
}


/// \brief  Slot for signal: data is available
///
///         receve it, parse it, execute it
bool CNetworkConfig::slotReceiveFunc( Glib::IOCondition condition )
{
    int size;
    (void)condition;

    Glib::RefPtr<Gio::SocketAddress> addr;
    size=receive( addr, buffer, maxBufferSize);
    parseBuffer(addr, size);

    return(true);
}


/// \brief  le destructeur
CNetworkConfig::~CNetworkConfig()
{
};


/// brief   initially read all data
///
///         Read config.
void CNetworkConfig::collectData()
{
    erase();
    getMacAddress(VISUX_INTERFACE, &mac);

    try
    {
        if( confHandler->getConfParameter( "lanConfigurationMode" )== "DHCP" )
            networkFlags=eNetworkFlagsDhcp;
        else
            networkFlags=0;

        getIpFromString(&ip,     confHandler->getConfParameter( "lanIPaddress" ) );
        getIpFromString(&subnet, confHandler->getConfParameter( "lanSubnetMask" ) );
        getIpFromString(&dns1,   confHandler->getConfParameter( "lanDNS1" ) );
        getIpFromString(&dns2,   confHandler->getConfParameter( "lanDNS2" ) );
    }
    catch( ... )
    {
        // config is cleared anyway
        PRINT_ERROR("CNetworkConfig::collectData: Could not read config\n");
    }

    try
    {
        getIpFromString(&gateway, confHandler->getConfParameter( "lanGateway" ) );
        webservice_port = getPortFromString(confHandler->getConfParameter( "lanWebservicePort" ) );
    }
    catch(...)
    {
        PRINT_ERROR("CNetworkConfig::collectData: Could not read config part 2\n");
        memset(&gateway, 0, sizeof(gateway));
        webservice_port = 0;
    }
}

void CNetworkConfig::collectNtpData()
{
    try
    {
        getIpFromString(&ntp_server, confHandler->getConfParameter( "lanTimeServer" ) );
    }
    catch(...)
    {
        PRINT_ERROR("CNetworkConfig::collectNtpData: Could not read ntp config\n");
        memset(&ntp_server, 0, sizeof(ntp_server));
    }
}

/// \brief  reads network configuration from ethernet adapter
///
///         This method has to be called after dhcp request in order to
///         have the actual network settings in the configuration file.
void CNetworkConfig::checkBackNetworkConfig()
{
    int sta=getNetworkAddress( VISUX_INTERFACE, &ip, &subnet );

    getDnsServer( &dns1, &dns2);

    getDefaultGateway ( &gateway );
    if(sta)
    {
        lError("Could not get network information from ethernet device (%d)\n", sta);
    }

    return;
}

/// \brief  Write data to config file
void CNetworkConfig::writeToConfig( bool ignoreChanges )
{
    Glib::ustring nullString;
    confHandler->clearConfigChangedHandlerMask();
    confHandler->setConfParameter( "macAddress", nullString, getStringFromMac( &mac ) );
    confHandler->setConfParameter( "lanConfigurationMode", nullString,(networkFlags==eNetworkFlagsDhcp)?"DHCP":"Manual" );
    confHandler->setConfParameter( "lanIPaddress", nullString, getStringFromIp( &ip ) );
    confHandler->setConfParameter( "lanSubnetMask", nullString, getStringFromIp( &subnet ) );
    confHandler->setConfParameter( "lanDNS1", nullString, getStringFromIp( &dns1 ) );
    confHandler->setConfParameter( "lanDNS2", nullString, getStringFromIp( &dns2 ) );
    confHandler->setConfParameter( "lanGateway", nullString, getStringFromIp( &gateway ) );
    confHandler->setConfParameter( "lanTimeServer", nullString, getStringFromIp( &ntp_server ) );
    confHandler->setConfParameter( "lanWebservicePort", nullString, Glib::ustring::compose("%1",webservice_port) );

    // We dont want to get signaled by our own changes. This is needed when we
    // updatenetwork information we got from DHCP so we must not resetup network
    // (infinite loop)
    if(ignoreChanges)
        confHandler->clearConfigChangedHandlerMask();

    confHandler->emitConfigChanged();
}

/// \brief  Write data to config file
void CNetworkConfig::write_to_config_net_only( bool ignoreChanges )
{
Glib::ustring nullString;
confHandler->clearConfigChangedHandlerMask();
confHandler->setConfParameter( "macAddress", nullString, getStringFromMac( &mac ) );
confHandler->setConfParameter( "lanConfigurationMode", nullString,(networkFlags==eNetworkFlagsDhcp)?"DHCP":"Manual" );
confHandler->setConfParameter( "lanIPaddress", nullString, getStringFromIp( &ip ) );
confHandler->setConfParameter( "lanSubnetMask", nullString, getStringFromIp( &subnet ) );
confHandler->setConfParameter( "lanDNS1", nullString, getStringFromIp( &dns1 ) );
confHandler->setConfParameter( "lanDNS2", nullString, getStringFromIp( &dns2 ) );
confHandler->setConfParameter( "lanGateway", nullString, getStringFromIp( &gateway ) );

// We dont want to get signaled by our own changes. This is needed when we
// updatenetwork information we got from DHCP so we must not resetup network
// (infinite loop)
if(ignoreChanges)
    confHandler->clearConfigChangedHandlerMask();

confHandler->emitConfigChanged();
}

/// \brief  erase all settings
void CNetworkConfig::erase()
{
    networkFlags=eNetworkFlagsDhcp;
    memset( ip, 0, sizeof(ip) );
    memset( subnet, 0, sizeof(subnet) );
    memset( dns1, 0, sizeof(dns1) );
    memset( dns2, 0, sizeof(dns2) );
    memset( gateway, 0, sizeof(gateway) );
    memset( ntp_server, 0, sizeof(ntp_server) );
    webservice_port=0;
}


/// \brief  reset all settings
///
///         like erase() but does not reset device name, serial, firmware
void CNetworkConfig::reset()
{
    networkFlags=eNetworkFlagsDhcp;
    memset( ip, 0, sizeof(ip) );
    memset( subnet, 0, sizeof(subnet) );
    memset( dns1, 0, sizeof(dns1) );
    memset( dns2, 0, sizeof(dns2) );
    memset( gateway, 0, sizeof(gateway) );
    memset( ntp_server, 0, sizeof(ntp_server) );
}


/// \brief  write rubbish into settings
///
///         For testing issues only.
void CNetworkConfig::waste()
{
    networkFlags=4;
    setIp( &ip, 1, 2, 3, 4 );
    //setIp( &subnet, 255, 255, 0, 0 );
    setIp( &subnet, 5, 6, 7, 8 );
    setIp( &dns1, 5, 6, 7, 8 );
    setIp( &dns2, 9, 10, 11, 12 );
    setIp( &gateway, 17, 18, 19, 20 );
    setIp( &ntp_server, 13,14,15,165456 );
    webservice_port=23456789;

}


/// \brief  write fixed IP into settings
///
///         For testing issues only.
void CNetworkConfig::fillStaticIp()
{
    networkFlags=eNetworkFlagsStatic;
    setIp( &ip, 192, 168, 0, 16 );
    setIp( &subnet, 255, 255, 255, 0 );
    setIp( &dns1, 192, 168, 0, 2 );
    setIp( &dns2, 192, 168, 0, 3 );
    setIp( &gateway, 192, 168, 0, 1 );
    setIp( &ntp_server, 192, 168, 0, 5 );
    webservice_port=8080;
}


int CNetworkConfig::parseBuffer(const Glib::RefPtr<Gio::SocketAddress>& addr, int size)
{
    if( size < 7 ) // e.g. "RDX SEA"
    {
        PRINT_ERROR("ERROR: Packet size to small");
        return(0);
    }

    if( ! memcmp(buffer, "RDX", 3) )
    {
        if( ! ( visuxMode == eVisuxModeDaemon) )
            // Skipping Requests in controller mode; probably send by myself
            return(0);

        if( !memcmp(&buffer[4], "SEA", 3) )
            handleSearchCommand(addr, size);
        else
        if( !memcmp(&buffer[4], "ASS", 3) )
            handleAssignCommand(addr, size);
        else
        if( !memcmp(&buffer[4], "RES", 3) )
            handleResetCommand(addr, size);
    }
    else
    if( ! memcmp(buffer, "rdx", 3) )
    {
        if( ! ( visuxMode == eVisuxModeController ) )
            // Skipping Requests in controller mode; probably send by myself
            return(0);

        if( !memcmp(&buffer[4], "sea", 3) )
            digestSearchResponse(size);
        else
        if( !memcmp(&buffer[4], "ass", 3) )
            digestAssignResponse(size);
        else
        if( !memcmp(&buffer[4], "res", 3) )
            digestResetResponse(size);
    }
    else
    {
        PRINT_ERROR("Can't hand input data");
    }

    return(0);
}


void fillArrayField(Glib::RefPtr<Glib::ByteArray> &arr, int field_len, int curr_len)
{
    static const unsigned char zeroes[]="\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0";

    if( curr_len < field_len )
        arr->append(zeroes, field_len - curr_len);

    return;
}

void CNetworkConfig::handleSearchCommand(
    const Glib::RefPtr<Gio::SocketAddress>& addr, int size )
{
    Glib::RefPtr<Glib::ByteArray> rsp=Glib::ByteArray::create();
    Glib::ustring webservice_port_string=Glib::ustring::compose("%1", webservice_port);
    int curr_len, field_len;
    ip_t real_ip, real_subnet, real_dns1, real_dns2;

    if( size != 8 )
        PRINT_ERROR("RDX size does not match: " <<  size);

    rsp->append((unsigned char *)"rdx sea ", 8);
    rsp->append((unsigned char *)&mac, sizeof(mac));
    rsp->append((unsigned char *)&networkFlags, sizeof(networkFlags));

    getNetworkAddress(VISUX_INTERFACE, &real_ip, &real_subnet);
    getDnsServer(&real_dns1, &real_dns2);

    rsp->append((unsigned char *)&real_ip, sizeof(real_ip));
    rsp->append((unsigned char *)&real_subnet, sizeof(real_subnet));
    rsp->append((unsigned char *)&real_dns1, sizeof(real_dns1));
    rsp->append((unsigned char *)&real_dns2, sizeof(real_dns2));

    rsp->append((unsigned char *)&ntp_server, sizeof(ntp_server));
    rsp->append((unsigned char *)webservice_port_string.c_str(),
                curr_len=MIN(webservice_port_string.bytes(), field_len= 5));
    fillArrayField(rsp, field_len, curr_len);

    Glib::ustring device_serial = confHandler->getConfParameter ("serialnumber");
    rsp->append((unsigned char *)device_serial.c_str(),
                curr_len=MIN( device_serial.bytes(), field_len = 10));
    fillArrayField(rsp, field_len, curr_len);

    Glib::ustring firmware_dc = confHandler->getConfParameter( "versionDC" );
    rsp->append((unsigned char *)firmware_dc.c_str(),
                curr_len=MIN( firmware_dc.bytes(), field_len = 10));
    fillArrayField(rsp, field_len, curr_len);


    Glib::ustring firmware_sic = confHandler->getConfParameter( "versionSIC" );
    rsp->append((unsigned char *)firmware_sic.c_str(),
                curr_len=MIN( firmware_sic.bytes(), field_len = 10));
    fillArrayField(rsp, field_len, curr_len);

    Glib::ustring room_identifier = confHandler->getConfParameter( "roomIdentifier" );
    rsp->append((unsigned char *)room_identifier.c_str(),
                curr_len=MIN( room_identifier.bytes(), field_len = 30));
    fillArrayField(rsp, field_len, curr_len);

    rsp->append( (unsigned char *)"\x04", 1 );

    send(addr, (const char *)rsp->get_data(), rsp->size());
}

void CNetworkConfig::handleAssignCommand(
    const Glib::RefPtr<Gio::SocketAddress>& addr, int size)
{
    std::stringstream strstr;
    SAssignCommand *cmd=(SAssignCommand *)buffer;
    Glib::RefPtr<Glib::ByteArray> rsp=Glib::ByteArray::create();

    PRINT_DEBUG("Got 'assign'-command");

    if ( size != ASSIGN_COMMAND_SIZE )
    {
        PRINT_ERROR("Wrong structure size");
        return;
    }

    if( memcmp(&mac, &cmd->mac, sizeof(mac) ) )
    {
        PRINT_DEBUG("Ignore assign command, its not for me");
        return;
    }

    memcpy(&networkFlags, &cmd->network_flags, sizeof(networkFlags));
    memcpy(&controlFlags, &cmd->control_flags, sizeof(controlFlags));
    memcpy(&ip, &cmd->ip, sizeof(ip));
    memcpy(&subnet, &cmd->subnet, sizeof(subnet));
    memcpy(&dns1, &cmd->dns1, sizeof(dns1));
    memcpy(&dns2, &cmd->dns2, sizeof(dns2));
    memcpy(&ntp_server, &cmd->ntp_server, sizeof(ntp_server));
    memcpy(&ntp_server, &cmd->ntp_server, sizeof(ntp_server));
    strstr.write(cmd->ws_port, 5);
    strstr >> webservice_port;

    // Ignore config change because i will update the network manually
    writeToConfig( true );

    if(verbose_network)
        info();

    rsp->append( (unsigned char *)"rdx ass ", 8);
    rsp->append( (unsigned char *)&mac, sizeof(mac) );

    // Send response with ACK(0x06)
    rsp->append( (unsigned char *)"\x06\x04", 2 );
    send(addr, (const char *)rsp->get_data(), rsp->size());

    updateNetwork();

    // inform DC
    replay_network_settings();
}

void CNetworkConfig::replay_network_settings(bool mode_only)
{
    std::vector<Glib::ustring> ids, values;

    auto mode = confHandler->getParameter("lanConfigurationMode");

    ids.emplace_back("lanConfigurationMode");
    values.emplace_back(mode);

    auto dc = QueryClient::get_instance();
    auto xq = XmlQueryReplayStatus::create(ids, values);

    assert(ids.size() == values.size());

    for (auto i = 0u; i < ids.size(); ++i)
        PRINT_DEBUG("Setting network parameter " << ids[i] << " to " <<
                    values[i] << " on DC");

    _state_query = dc->create_query(xq);
    _state_query->finished.connect(
        sigc::bind<bool>(
            sigc::mem_fun(*this, &CNetworkConfig::on_set_mode_parameter_finished),
            mode_only));
    dc->execute(_state_query);
}

void CNetworkConfig::handleResetCommand(
    const Glib::RefPtr<Gio::SocketAddress>& addr, int size)
{
    int sta=0;
    SResetCommand *cmd=(SResetCommand *)buffer;
    Glib::RefPtr<Glib::ByteArray> rsp=Glib::ByteArray::create();

    PRINT_DEBUG("Got 'reset'-command");

    if ( size != RESET_COMMAND_SIZE )
    {
        PRINT_ERROR("Wrong structure size");
        return;
    }

    if( memcmp(&mac, &cmd->mac, sizeof(mac) ) )
    {
        PRINT_DEBUG("Ignore reset command, its not for me");
        return;
    }

    reset();
    sta=updateNetwork();

    if(verbose_network)
        info();

    rsp->append( (unsigned char *)"rdx res ", 8 );
    rsp->append( (unsigned char *)&mac, sizeof(mac) );

    // Send response with ACK(0x06) or with NAK (0x15)
    rsp->append( (unsigned char *) ( (sta==0) ? "\x06\x04" : "\x15\x04"), 2 );

    send(addr, (const char *)rsp->get_data(), rsp->size());

    // inform DC
    replay_network_settings(true);
}


void CNetworkConfig::digestSearchResponse( int size )
{
    std::stringstream strstr;
    SSearchResponse *rsp=(SSearchResponse *)buffer;

    PRINT_DEBUG("Got 'search'-response");

    dump(buffer, size);

    if ( size != SEARCH_RESPONSE_SIZE )
    {
        PRINT_ERROR("Wrong structure size");
        return;
    }

    memcpy(&mac, &rsp->mac, sizeof(mac));
    memcpy(&networkFlags, &rsp->flags, sizeof(networkFlags));
    memcpy(&ip, &rsp->ip, sizeof(ip));
    memcpy(&subnet, &rsp->subnet, sizeof(subnet));
    memcpy(&dns1, &rsp->dns1, sizeof(dns1));
    memcpy(&dns2, &rsp->dns2, sizeof(dns2));
    memcpy(&ntp_server, &rsp->ntp_server, sizeof(ntp_server));
    memcpy(&ntp_server, &rsp->ntp_server, sizeof(ntp_server));
    strstr.write(rsp->ws_port, 5);
    strstr >> webservice_port;

    if(verbose_network)
        info();
}


void CNetworkConfig::digestAssignResponse( int size )
{
    (void)size;
    PRINT_DEBUG("Got 'assign'-response");
}


void CNetworkConfig::digestResetResponse( int size )
{
    (void)size;
    PRINT_DEBUG("Got 'reset'-response");
}


void CNetworkConfig::info(void)
{
    printf("Network flags: %d\n", networkFlags);
    printf("MAC: ");
    dumpMac(&mac);
    printf("\n");

    printf("IP / Subnet: ");
    dumpIp(&ip);
    printf(" / ");
    dumpIp(&subnet);
    printf("\n");

    printf("DNS 1/2: ");
    dumpIp(&dns1);
    printf(" / ");
    dumpIp(&dns2);
    printf("\n");

    printf("NTP Server: ");
    dumpIp(&ntp_server);
    printf("\n");

    printf("Gateway: ");
    dumpIp(&gateway);
    printf("\n");

    printf("Webservice-Port: %d\n", webservice_port);
}


int CNetworkConfig::runCommand( const Glib::ustring &command)
{
    int app_status;

    std::string standard_output;
    std::string standard_error;
    std::vector <std::string> argv = Glib::Regex::split_simple(" ", command);

    PRINT_DEBUG("Command: " << command);

    try
    {
        Glib::spawn_sync( "" /*PWD*/, Glib::ArrayHandle< std::string >(argv)
                      , Glib::SPAWN_DEFAULT, sigc::slot<void>(), &standard_output, &standard_error, &app_status
            /*SpawnFlags flags=SPAWN_DEFAULT, const SlotSpawnChildSetup& child_setup=SlotSpawnChildSetup(), std::string* standard_output=nullptr, std::string* standard_error=nullptr, int* exit_status=nullptr*/
            );
    }
    catch ( const Glib::SpawnError &ex )
    {
        throw std::logic_error( ex.what() );
    }

    PRINT_DEBUG("Status : " << app_status);
    PRINT_DEBUG("stdout : " << standard_output);
    PRINT_DEBUG("stderr : " << standard_error);

    return( app_status );
}


int CNetworkConfig::updateNetwork()
{
    int sta=0;

    PRINT_DEBUG("Updating network");

    #if ! defined ( GLOBAL_INSTALLATION )
        {
            PRINT_DEBUG("Not updating network, i'm not root or not allowd");
        }
    #else
        runCommand( "/sbin/route del -net 255.255.255.255 netmask 255.255.255.255" );
        runCommand( Glib::ustring::compose( "/sbin/ifdown %1", VISUX_INTERFACE ) );
        runCommand( Glib::ustring::compose( "/sbin/ifconfig %1 down", VISUX_INTERFACE ) );

        updateNetworkConfig();

        runCommand( Glib::ustring::compose( "/sbin/ifup %1", VISUX_INTERFACE ) );
        runCommand( Glib::ustring::compose( "/sbin/ifconfig %1 up", VISUX_INTERFACE ) );
        runCommand( Glib::ustring::compose( "/sbin/route add -net 255.255.255.255 netmask 255.255.255.255 %1" , VISUX_INTERFACE ) );
    #endif

    if ( ( networkFlags & eNetworkFlagsDhcp ) )
    {
        checkBackNetworkConfig();
        write_to_config_net_only( true /* ignoreChanges */ );
        if(verbose_network)
            info();
    }

    return(sta);
}

int CNetworkConfig::updateNtpConfig ()
{
#if ! defined ( GLOBAL_INSTALLATION )
    PRINT_DEBUG("Not updating ntp config, i'm not root or not allowed");
#else
    Glib::RefPtr<Glib::IOChannel> file;

    runCommand( "/etc/init.d/ntp stop" );

    file=Glib::IOChannel::create_from_file("/etc/ntp.conf", "w");
    file->write( Glib::ustring::compose( "server %1\n", getStringFromIp( &ntp_server ) ) );
    file->write( Glib::ustring::compose( "restrict %1\n", getStringFromIp( &ntp_server ) ) );
    file->write( "restrict 127.0.0.1\n");
    file->write( "driftfile /opt/transfer/ntp/ntp.drift\n");
    file->close();

    runCommand( "/etc/init.d/ntp start" );
#endif
    return 0;
}

bool
CNetworkConfig::reread_network_config ()
{
    if ( ( networkFlags & eNetworkFlagsDhcp ) )
    {
        checkBackNetworkConfig();
        write_to_config_net_only( true /* ignoreChanges */ );
    }

    return true;
}


int CNetworkConfig::updateNetworkConfig()
{
    Glib::RefPtr<Glib::IOChannel> file=Glib::IOChannel::create_from_file("/etc/network/interfaces.d/zix.conf", "w");

    auto host_name = confHandler->getParameter("deviceHostname");

    if ( ( networkFlags & eNetworkFlagsDhcp ) )
    {
        file->write( Glib::ustring::compose( "allow-hotplug %1\n", VISUX_INTERFACE ) );
        file->write( Glib::ustring::compose( "auto %1\n", VISUX_INTERFACE ) );
        file->write( Glib::ustring::compose( "iface %1 inet manual\n", VISUX_INTERFACE ) );
        if (host_name.empty())
            file->write( Glib::ustring::compose( "    pre-up /sbin/udhcpc -t 15 -b -p /run/udhcpc.%1.pid -i %1 \n", VISUX_INTERFACE, VISUX_INTERFACE ) );
        else
            file->write( Glib::ustring::compose( "    pre-up /sbin/udhcpc -x hostname:%1 -t 10 -b -p /run/udhcpc.%2.pid -i %2 \n", host_name, VISUX_INTERFACE ) );
        file->write( Glib::ustring::compose( "    pre-down kill -USR2 `cat /run/udhcpc.%1.pid`; kill -TERM `cat /run/udhcpc.%1.pid`\n", VISUX_INTERFACE, VISUX_INTERFACE ) );
	file->write( "    ethernet-autoneg 0x0f\n" );
    }
    else
    {
        file->write( Glib::ustring::compose( "allow-hotplug %1\n", VISUX_INTERFACE ) );
        file->write( Glib::ustring::compose( "auto %1\n", VISUX_INTERFACE ) );
        file->write( Glib::ustring::compose( "iface %1 inet static\n", VISUX_INTERFACE ) );
        file->write( Glib::ustring::compose( "    address %1\n", getStringFromIp( &ip ) ) );
        file->write( Glib::ustring::compose( "    netmask %1\n", getStringFromIp( &subnet ) ) );
        file->write( Glib::ustring::compose( "    dns-nameservers %1 %2\n"
                                             , getStringFromIp( &dns1 )
                                             , getStringFromIp( &dns2 ) ) );
        file->write( Glib::ustring::compose( "    gateway %1\n", getStringFromIp( &gateway ) ) );
	file->write( "    ethernet-autoneg 0x0f\n" );
    }
    file->close();


    if ( ! ( networkFlags & eNetworkFlagsDhcp ) )
    {
        file=Glib::IOChannel::create_from_file("/etc/resolv.conf", "w");
        file->write( "# This file was created by libzix, don't change it manually.\n");
        file->write( "\n");

        if( ! isEmptyIp(&dns1) )
            file->write( Glib::ustring::compose( "nameserver %1\n", getStringFromIp( &dns1 ) ) );
        if( ! isEmptyIp(&dns2) )
            file->write( Glib::ustring::compose( "nameserver %1\n", getStringFromIp( &dns2 ) ) );
        file->close();
    }

    return(0);
}


void CNetworkConfig::slot_conf_change_announce(
    const Glib::ustring& par_id, const Glib::ustring& value, int &handlerMask)
{
    UNUSED(value);

    if ( ( par_id == "lanConfigurationMode" )
           || ( par_id == "lanIPaddress" )
           || ( par_id == "lanSubnetMask" )
           || ( par_id == "lanDNS1" )
           || ( par_id == "lanDNS2" )
           || ( par_id == "lanGateway" )
           || ( par_id == "lanWebservicePort" )
	   || ( par_id == "all" ) )
        handlerMask |= HANDLER_MASK_NETWORK;

    if ( ( par_id == "lanTimeServer" )
	   || ( par_id == "all" ) )
	handlerMask |= HANDLER_MASK_NTPCONF;
}


void CNetworkConfig::slot_conf_changed( int handlerMask )
{
    if( handlerMask & HANDLER_MASK_NETWORK )
    {
        lDebug("Updating network config\n");
        collectData();
        updateNetwork();
    }

    if ( handlerMask & HANDLER_MASK_NTPCONF )
    {
	collectNtpData ();
	updateNtpConfig ();
    }
}

bool
CNetworkConfig::online ()
{
    return ! isEmptyIp (& ip);
}

void CNetworkConfig::on_set_mode_parameter_finished(const Glib::RefPtr<XmlResult>& result, bool mode_only)
{
    if (result->get_status() != 200)
        PRINT_ERROR("Failed to set LAN mode on DC");

    std::vector<Glib::ustring> ids, values;

    auto mode = confHandler->getParameter("lanConfigurationMode");

    // Do we have to update the network settings on DC?
    if (mode_only)
        return;

    // Always update webservice port
    auto port = confHandler->getConfParameter("lanWebservicePort");
    ids.emplace_back("lanWebservicePort");
    values.emplace_back(port);

    // And additionally the network parameters
    if (mode == "Manual") {
        auto ip      = confHandler->getParameter("lanIPaddress");
        auto mask    = confHandler->getParameter("lanSubnetMask");
        auto dns1    = confHandler->getParameter("lanDNS1");
        auto dns2    = confHandler->getParameter("lanDNS2");
        auto gateway = confHandler->getParameter("lanGateway");
        auto time    = confHandler->getParameter("lanTimeServer");

        ids.emplace_back("lanIPaddress");
        ids.emplace_back("lanSubnetMask");
        ids.emplace_back("lanDNS1");
        ids.emplace_back("lanDNS2");
        ids.emplace_back("lanGateway");
        ids.emplace_back("lanTimeServer");

        values.emplace_back(ip);
        values.emplace_back(mask);
        values.emplace_back(dns1);
        values.emplace_back(dns2);
        values.emplace_back(gateway);
        values.emplace_back(time);
    }

    auto dc = QueryClient::get_instance();
    auto xq = XmlQueryReplayStatus::create(ids, values);

    assert(ids.size() == values.size());

    for (auto i = 0u; i < ids.size(); ++i)
        PRINT_DEBUG("Setting network parameter " << ids[i] << " to " <<
                    values[i] << " on DC");

    _state_query = dc->create_query(xq);
    _state_query->finished.connect(sigc::mem_fun(*this, &CNetworkConfig::on_set_lan_parameters_finished));
    dc->execute(_state_query);
}

void CNetworkConfig::on_set_lan_parameters_finished(const Glib::RefPtr<XmlResult>& result)
{
    if (result->get_status() != 200)
        PRINT_ERROR("Failed to set network parameters on DC");
}

//---fin.----------------------------------------------------------------------
