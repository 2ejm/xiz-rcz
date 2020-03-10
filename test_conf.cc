
#include "monitor_manager.h"
#include "monitor.h"
#include "monitor_cpu.h"
#include "monitor_memory.h"
#include "monitor_network.h"
#include "monitor_directory.h"
#include "monitor_uptime.h"
#include "conf_handler.h"
#include "serial_interface_handler.h"
#include "zix_interface.h"

#include "xml_processor.h"

//#include <glibmm/main.h>
#include <glibmm/init.h>
#include <giomm/init.h>


void testDirectories( const Glib::ustring &id )
{
    Glib::ustring directory;

    auto conf = ConfHandler::get_instance();

    directory=conf->getDirectory( id );

    if( ! directory.empty() )
        printf("### Directory '%s' found: %s\n", id.c_str(), directory.c_str() );
    else
        printf("### Directory not found (%s)\n", id.c_str() );
}


void testGetConf()
{
    Glib::ustring directory;
    Glib::ustring unit;
    Glib::ustring value;
    Glib::ustring nodes;

    auto conf = ConfHandler::get_instance();

    // getConfV2( item, id, resource, postProcessor)
    //nodes=conf->getConf(Glib::ustring(), Glib::ustring(), Glib::ustring(), Glib::ustring());
    //nodes=conf->getConf("parameter", Glib::ustring(), Glib::ustring(), Glib::ustring());
    //nodes=conf->getConf("allocationProcessor", Glib::ustring(), Glib::ustring(), Glib::ustring());
    //nodes=conf->getConf("format", Glib::ustring(), Glib::ustring(), Glib::ustring());
    //nodes=conf->getConf("printer", Glib::ustring(), Glib::ustring(), Glib::ustring());
    //nodes=conf->getConf("logo", Glib::ustring(), Glib::ustring(), Glib::ustring());
    //nodes=conf->getConf("protocol", Glib::ustring(), Glib::ustring(), Glib::ustring());
    //nodes=conf->getConf("output", Glib::ustring(), Glib::ustring(), Glib::ustring());

    //nodes=conf->getConf("parameter", "serialParity", Glib::ustring(), Glib::ustring());
    //nodes=conf->getConf("parameter", "lanSocket", Glib::ustring(), Glib::ustring());

    //nodes=conf->getConf("parameter", Glib::ustring(), "lanFolder", Glib::ustring());
    //nodes=conf->getConf("parameter", Glib::ustring(), Glib::ustring(), "PDF");
    //nodes=conf->getConf("parameter", Glib::ustring(), "lanSocket", Glib::ustring());
    // nodes=conf->getConf("format", Glib::ustring(), "lanFolder", Glib::ustring());

    //nodes=conf->getConf("date", Glib::ustring(), Glib::ustring(), Glib::ustring());
    //nodes=conf->getConf("time", Glib::ustring(), Glib::ustring(), Glib::ustring());

    //nodes=conf->getConf("parameter", "RP1", Glib::ustring(), Glib::ustring());
    //nodes=conf->getConf("parameter", Glib::ustring(), "a", Glib::ustring());
    //nodes=conf->getConf( "name", Glib::ustring(), Glib::ustring(), "PDF");
    //nodes=conf->getConfNode("*/postProcessor[@id=TXT]");
    //nodes=conf->getConf(Glib::ustring(), Glib::ustring(), "serialCOM2", Glib::ustring());
    //nodes=conf->getConfNode(".//postProcessor[@id='TXT']");

    //nodes=conf->getConf("protocol", Glib::ustring(), "serial", Glib::ustring());

    printf("### nodes %s\n",  nodes.c_str() );

    return;
}


void testSetConf()
{
    auto conf = ConfHandler::get_instance();

    conf->setConfParameter("serialBaudrate", "Bd", "666");
}


int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    Glib::init();
    Gio::init();

    /* libzix init function...
     * TODO: migrate into a single libzix::init()
     */

    //auto settings = ConfHandler::create();
    //settings->getPostprocessor( "dummy", " );
    //settings->getConf("");

    /*
    testPostprocessor("LAN:printer", "XML", "PDF");
    testPostprocessor("LAN:printer_X", "XML", "PDF");
    testPostprocessor("LAN:printer", "XML_X", "PDF");
    testPostprocessor("LAN:printer", "XML", "PDF_X");
    testPostprocessor("LAN:printer_X", "XML_X", "PDF_X");
    */
    /*

    testDirectories( "measurements" );
    testDirectories( "monitor" );
    testDirectories( "monitor_blaah" );
    testDirectories( "monitor1" );
    testDirectories( "monitor2" );

    */
    //testGetConf();
    testSetConf();


    exit(22);
    auto xml_p = XmlProcessor::create ();
    auto hand = SerialInterfaceHandler::create( STR_ZIXINF_COM1, "/dev/ttyUSB0" , 115800, xml_p
                                                ,ESerialHandlerModeNormal );
    testGetConf();

    #ifdef DEBUG_CLASSES
        Postprocessor::debugInfo();
        Database::debugInfo();
    #endif

    Glib::RefPtr<Glib::MainLoop> mainloop = Glib::MainLoop::create();
    mainloop->run();
}
