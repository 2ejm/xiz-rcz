//----------------------------------------------------------------------------
///
/// \file   test_monitor.cc
///
/// \brief  Test monitor alarms
///
//----------------------------------------------------------------------------


#include "monitor_manager.h"
#include "monitor.h"
#include "monitor_cpu.h"
#include "monitor_memory.h"
#include "monitor_network.h"
#include "monitor_directory.h"
#include "monitor_uptime.h"

//#include <glibmm/main.h>
#include <glibmm/init.h>
#include <giomm/init.h>



int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    Glib::init();
    Gio::init();

    auto monitorManager = MonitorManager::get_instance();
    monitorManager->addMonitor( Monitor::create("dummy") );
    monitorManager->addMonitor( MonitorCpu::create() );
    monitorManager->addMonitor( MonitorNetwork::create() );
    monitorManager->addMonitor( MonitorMemory::create() );
    monitorManager->addMonitor( MonitorDirectory::create( "/tmp" ) );
    monitorManager->addMonitor( MonitorUptime::create(  ) );

    monitorManager->findMonitor( "Directory: /tmp" )->setAlarmThresholds( 23 );
    monitorManager->findMonitor( "CPU" )->setAlarmThresholds( 10, 10, eAlarmSlopeTypeFalling );

    Glib::RefPtr<Glib::MainLoop> mainloop = Glib::MainLoop::create();
    mainloop->run();
}


//---fin.---------------------------------------------------------------------
