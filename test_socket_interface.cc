
#include "socket_interface_handler.h"
#include "hexio.h"
#include "xml_processor.h"
#include "zix_interface.h"

#include "core_function_exit.h"

#include <glibmm/main.h>
#include <glibmm/init.h>
#include <giomm/init.h>



int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    /* Glib Gio init functions...
     * Strange things happen, if these are not
     * called...
     *
     * XXX: GThreads init ?
     */
    Glib::init();
    Gio::init();

    /* libzix init function...
     * TODO: migrate into a single libzix::init()
     */
    CoreFunctionCall::init();
    XmlParameter::init();

    auto xml_p = XmlProcessor::create ();

    auto hand = SocketInterfaceHandler::create("Unknown", 17001, xml_p);
    InterfaceHandler::register_handler (hand);

    CoreFunctionExit::mainloop = Glib::MainLoop::create();
    CoreFunctionExit::mainloop->run();

    CoreFunctionExit::mainloop.reset();
}
