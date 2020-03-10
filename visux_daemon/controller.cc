//----------------------------------------------------------------------------
///
/// \file   controller.cc
///
/// \brief  test visux daemon
///
///         This is just a test application. It is the counterpart for the visux
///         daemon. You can send commands to the clients which should respond.
///
///         Sends "search"/"assign"/"reset"
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//----------------------------------------------------------------------------


//---Includes-----------------------------------------------------------------


//---General-------------------------

#include <glibmm.h>
#include <iostream>
#include <string.h>
#include <giomm/init.h>

//---Own-----------------------------

#include "visux.h"


//---Implementation-----------------------------------------------------------


Glib::RefPtr<Glib::MainLoop> loop;


int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    mac_t laptop_mac={ 0x00, 0x26, 0x2D, 0xF5, 0x56, 0x2D };
    Gio::init();
    //CUdpChannel udpChannel;
    CVisux visux ( eVisuxModeController );

    loop = Glib::MainLoop::create();
    (void)laptop_mac;
    /*
    visux.setWebservicePort(12345);
    visux.setDeviceName( "MyLittleDevice" );
    visux.setFirmwareDc( "V1.0" );
    visux.setFirmwareSic( "AlleMeineEntchenSchwimmenAufDemSee" );
    */


    //visux.erase( );
    //visux.sendSearchCommand();

    visux.setMac( laptop_mac );

    //visux.waste();
    //visux.sendAssignCommand();

    //visux.reset();
    //visux.sendAssignCommand();

    visux.sendResetCommand();


    loop->run();

    return 0;
}


//---fin.---------------------------------------------------------------------
