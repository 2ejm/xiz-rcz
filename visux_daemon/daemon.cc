//----------------------------------------------------------------------------
///
/// \file   daemon.cc
///
/// \brief  VisuX daemon
///
///         Just waits for broadcast messages to be Answerden
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//----------------------------------------------------------------------------


//---Includes-----------------------------------------------------------------


//---General-------------------------

#include <giomm.h>
#include <glibmm.h>

//---Own-----------------------------

#include "visux.h"


//---Implementation-----------------------------------------------------------


Glib::RefPtr<Glib::MainLoop> loop;


int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    Gio::init();

    loop = Glib::MainLoop::create();

    CVisux visux( eVisuxModeDaemon );

    loop->run();

    return 0;
}

//---fin.---------------------------------------------------------------------
