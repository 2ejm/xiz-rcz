//-----------------------------------------------------------------------------
///
/// \brief  Libvisux daemon
///
///         just instantiates the handler in jumps to mainloop.
///
/// \date   [20161221] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <glibmm/main.h>
#include <glibmm/init.h>
#include <giomm/init.h>

//---Own------------------------------

#include "libzix.h"

#include "core_function_exit.h"


//---Implementation------------------------------------------------------------


int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    /* Glib Gio init functions...
     */
    Glib::init();
    Gio::init();
    libzix::init();

    CoreFunctionExit::mainloop = Glib::MainLoop::create();
    CoreFunctionExit::mainloop->run();

    CoreFunctionExit::mainloop.reset();
}


//---fin.----------------------------------------------------------------------
