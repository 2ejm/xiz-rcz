//----------------------------------------------------------------------------
///
/// \brief  Test serial interface handler
///
///         There are three modes:
///             * regulare mode; just run as normal daemon
///             * counterpart mode; send command to serial interface. The
///               daemon has to run
///             * devicecontroler mode; play the role of a devicecontroller;
///               daemon has to run, the daemon can send commands to
///               "devicecontroller"
///
///         The "regular mode" runs on /dev/ttyUSB0, the others on /dev/ttyUSB1.
///
/// \date   [20161129] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//----------------------------------------------------------------------------


#include "socket_interface_handler.h"
#include "serial_interface_handler.h"
#include "usbserial_interface_handler.h"
#include "hexio.h"
#include "xml_processor.h"
//#include "_crc.h"
#include "crc32.h"
#include "log.h"
#include "zix_interface.h"

#include "core_function_exit.h"

#include <glibmm/main.h>
#include <glibmm/init.h>
#include <giomm/init.h>
#include <string.h>

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    ESerialHandlerMode eSerialHandlerMode = ESerialHandlerModeNormal;
    Glib::init();
    Gio::init();

    CoreFunctionCall::init();
    XmlParameter::init();


    //Glib::RefPtr<SerialInterfaceHandler> hand;

    printf("[%s]\n", basename( argv[0]));


    auto xml_p = XmlProcessor::create ();

    if ( strcmp( basename( argv[0]), "counterpart") == 0 )
        eSerialHandlerMode = ESerialHandlerModeCounterpart;

    if ( strcmp( basename( argv[0]), "counterpart2") == 0 )
        eSerialHandlerMode = ESerialHandlerModeCounterpart2;

    if( strcmp( basename( argv[0]), "devicecontroller") == 0 )
        eSerialHandlerMode = ESerialHandlerModeDeviceController;

    //try{
    //auto hand = UsbInterfaceHandler::create(STR_ZIXINF_IPC,
    auto hand = SerialInterfaceHandler::create(STR_ZIXINF_IPC,
                 ( eSerialHandlerMode != ESerialHandlerModeNormal )
                                               ? "/dev/ttyUSB0" : "/dev/ttyUSB1"
                , 115800, xml_p, eSerialHandlerMode);
                //, 9600, xml_p, eSerialHandlerMode);

    InterfaceHandler::register_handler(hand);
        /*
    }
    catch( ... )
    {
        printf("Error: could not create serial interface handler\n");
        exit(22);
    }*/

    setInternLogLevel( ELogLevelVeryHighDebug );

    switch( eSerialHandlerMode )
    {
        case ESerialHandlerModeCounterpart:
            lInfo("Counterpart mode\n");
            break;
        case ESerialHandlerModeCounterpart2:
            lInfo("Counterpart II mode\n");
            break;
        case ESerialHandlerModeDeviceController:
            lInfo("Device-Controller mode\n");
            break;
        default:
            lInfo("Normal mode\n");
            break;
    }

    if( eSerialHandlerMode == ESerialHandlerModeCounterpart )
    {
        //hand->injectWrongCrc();
        lInfo("Sending test message\n");
        //hand->sendTestMessage( "/home/mase/wp/prj/zeiss/calza/src/libzix/testxmls/get_env_given.xml" );
        //hand->sendTestMessage( "/home/mase/wp/prj/zeiss/calza/src/libzix/testxmls/get_conf_lan.xml" );
        hand->sendTestMessage( "/home/mase/wp/prj/zeiss/calza/src/libzix/testxmls/set_conf_loglevel.xml" );
        //hand->sendTestMessage( "/home/mase/wp/prj/zeiss/calza/src/libzix/testxmls/crc_issue.xml" );
        //        Glib::IOChannel::create_from_file("testxmls/get_env_named.xml", "r");
        //        Glib::IOChannel::create_from_file("testxmls/get_env_all.xml", "r");
        //        Glib::IOChannel::create_from_file("testxmls/get_env_given.xml", "r");
        //        Glib::IOChannel::create_from_file("testxmls/get_env_zix.xml", "r");
        //        Glib::IOChannel::create_from_file("testxmls/set_env_node.xml", "r");
        //        Glib::IOChannel::create_from_file("testxmls/set_env_given.xml", "r");
        //        Glib::IOChannel::create_from_file("testxmls/set_env_given_multiple.xml", "r");
        //        Glib::IOChannel::create_from_file("testxmls/set_env_erroneous_1.xml", "r");
    }


    printf("Main Loop\n");
    CoreFunctionExit::mainloop = Glib::MainLoop::create();
    CoreFunctionExit::mainloop->run();

    CoreFunctionExit::mainloop.reset();
}


//---fin----------------------------------------------------------------------
