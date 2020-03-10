
#include "monitor_manager.h"
#include "monitor.h"
#include "monitor_cpu.h"
#include "monitor_memory.h"
#include "monitor_network.h"
#include "monitor_directory.h"
#include "monitor_uptime.h"
#include "conf_handler.h"
#include "serial_interface_handler.h"
#include "socket_client.h"
#include "xml_processor.h"
#include <glibmm/init.h>
#include <giomm/init.h>


int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    int sta;

    Glib::init();
    Gio::init();

    auto hand = SocketClient::create( 17001);

    Glib::ustring response;

    #if 0
        sta=hand->getConf("parameter", Glib::ustring() , Glib::ustring(), Glib::ustring(), response);
        printf("Response text: %s\n", response.c_str() );
    #elif 0
        bool dhcp;
        Glib::ustring ip, netmask, dns1, dns2, gateway, ntp;
        sta=hand->getConfNetwork( dhcp, ip , netmask, dns1, dns2, gateway, ntp);
    #elif 0
        printf("Sending 'setFile DC' to daemon.\n");
        sta=hand->setFile( "DC", "data.dat", "DATA" );
    #elif 1
        printf("Sending 'getConf' to daemon.\n");
        sta=hand->getConf("parameter", Glib::ustring() , Glib::ustring(), Glib::ustring(), response);
        printf("Response text: %s\n", response.c_str() );
    #endif

    printf("Response status: %d\n", sta );


    Glib::RefPtr<Glib::MainLoop> mainloop = Glib::MainLoop::create();
    mainloop->run();
}
