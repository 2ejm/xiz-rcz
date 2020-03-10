//-----------------------------------------------------------------------------
///
/// \brief  Libvisux daemon
///
///         instantiates the handlers, parsers, monitors and sets up
///         configurations
///
/// \date   [20161221] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <glib.h>
#include <glibmm/refptr.h>

#include <stdexcept>

//---Own------------------------------

#include "libzix.h"
#include "socket_interface_handler.h"
#include "serial_interface_handler.h"
#include "usbserial_interface_handler.h"
#include "file_interface_handler.h"
#include "unix_socket_interface_handler.h"
#include "config_serial_interface_handler.h"
#include "xml_processor.h"
#include "monitor_manager.h"
#include "monitor_cpu.h"
#include "monitor_memory.h"
#include "monitor_network.h"
#include "monitor_uptime.h"
#include "monitor_directory.h"
#include "monitor_disk_usage.h"
#include "zix_interface.h"
#include "utils.h"
#include "core_function_call.h"
#include "conf_handler.h"
#include "file_handler.h"
#include "disk_usage_manager.h"
#include "samba_mounter.h"
#include "printer_monitor.h"
#include "webservice.h"
#include "state_variable.h"
#include "lan_socket_monitor.h"
#include "ntp_monitor.h"
#include "log_truncate_handler.h"

#ifdef GLOBAL_INSTALLATION
    #define USB_SERIAL_DEVICE   "/dev/usbserial"
    #define SERIAL_DEVICE       "/dev/ttyO4"
    #define SERIAL_COM1_DEVICE  "/dev/ttyO1"
    #define SERIAL_DEBUG_DEVICE "/dev/ttyO0"
#else
    //#define USB_SERIAL_DEVICE   "/dev/ttyUSB0"
    #define USB_SERIAL_DEVICE   "/dev/ttyUSB1"
    #define SERIAL_DEVICE       "/dev/ttyO4"
    #define SERIAL_COM1_DEVICE  "/dev/ttyO1"
    #define SERIAL_DEBUG_DEVICE "/dev/ttyO0"
#endif

//---Implementation------------------------------------------------------------


namespace libzix
{


Glib::RefPtr<XmlProcessor> xmlProcessor;
Glib::RefPtr<SocketInterfaceHandler> handlerDebug;
Glib::RefPtr<SerialInterfaceHandler> handlerSerial;
Glib::RefPtr<UsbInterfaceHandler> handlerUsbSerial;
Glib::RefPtr<UnixSocketInterfaceHandler> handlerWebService;
Glib::RefPtr<UnixSocketInterfaceHandler> handlerWebServer;
Glib::RefPtr<FileInterfaceHandler> handlerFileLAN;
Glib::RefPtr<FileInterfaceHandler> handlerFileUSB;
Glib::RefPtr<MonitorManager> monitorManager;
Glib::RefPtr<WatchdogManager> watchdogManager;
Glib::RefPtr<DataFreeHandler> dataFreeHandler;
Glib::RefPtr<DataFreeHandler> dataFreeHandler2;
Glib::RefPtr<LogTruncateHandler> logTruncateHandler;
Glib::RefPtr<CNetworkConfig> networkConfig;
Glib::RefPtr<ConfigSerialInterfaceHandler> configSerialHandler;
Glib::RefPtr<ConfigSerialInterfaceHandler> configDebugHandler;
Glib::RefPtr<SambaMounter> sambaMounter;
Glib::RefPtr<Webservice> webservice;

/* Prototypes
 */
void query_booting_finished (const Glib::RefPtr <XmlResult> & result);
void query_ready_finished (const Glib::RefPtr <XmlResult> & result);
void on_samba_mount_finished (bool mounted);
int init_part2 ();
int init_part3 ();

#ifdef GLOBAL_INSTALLATION
#define UNIX_SOCKET_PREFIX "/run/libzix"
#else
#define UNIX_SOCKET_PREFIX "."
#endif

StateVariable sic_state ("statusSIC");

static sigc::connection samba_mount_connection;

bool
run_query_booting ()
{
    sic_state.set_state ("booting", sigc::ptr_fun (query_booting_finished));

    return false;
}

bool
run_query_ready ()
{
    sic_state.set_state ("ready", sigc::ptr_fun (query_ready_finished));

    return false;
}

void
glib_log_func (const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
    (void) user_data;

    enum ELogLevel zix_level;

    if (log_level & G_LOG_LEVEL_ERROR)
	zix_level = ELogLevelError;
    else if (log_level & G_LOG_LEVEL_CRITICAL)
	zix_level = ELogLevelError;
    else if (G_LOG_LEVEL_WARNING)
	zix_level = ELogLevelWarn;
    else if (G_LOG_LEVEL_MESSAGE)
	zix_level = ELogLevelInfo;
    else if (G_LOG_LEVEL_INFO)
	zix_level = ELogLevelInfo;
    else if (G_LOG_LEVEL_DEBUG)
	zix_level = ELogLevelDebug;

    logger (zix_level, "glib domain: %s message: %s\n", log_domain, message);
}

int init()
{
    StateVariable::set_master (&sic_state);

    // create log instance so internal log is switched to libzix mechanism
    LogHandler::get_instance();

    /* setup glib logging, so that it uses our internal logging
     */
    g_log_set_default_handler (&glib_log_func, NULL);


    // now call the internal register functions
    CoreFunctionCall::init();
    XmlParameter::init();

    // xmlProcessor is needed for any InterfaceHandler
    xmlProcessor = XmlProcessor::create ();

    /* Create InterfaceHandler for Connection to DC
     * we want to tell DC aboout us
     */
    try {
        handlerUsbSerial = UsbInterfaceHandler::create(
            STR_ZIXINF_IPC, USB_SERIAL_DEVICE, 115800, xmlProcessor, ESerialHandlerModeNormal);
    }
    catch ( ... )
    {
        PRINT_ERROR("Could not install serial-usb interface " USB_SERIAL_DEVICE);
    }

    // -> IPC via RS422; only if USB is not available
    if(!handlerUsbSerial)
    {
        try {
            handlerSerial = SerialInterfaceHandler::create(
                STR_ZIXINF_IPC, SERIAL_DEVICE, 115800, xmlProcessor, ESerialHandlerModeNormal);
        }
        catch ( ... )
        {
            PRINT_ERROR("Could not install serial interface " SERIAL_DEVICE);
        }
    }

    /* register handlers, so that the Query Code can find them
     */
    if(handlerSerial)
    {
        InterfaceHandler::register_handler(handlerSerial);
    }
    if(handlerUsbSerial)
    {
        InterfaceHandler::register_handler(handlerUsbSerial);
    }

    run_query_booting ();

    return 0;
}

void
query_booting_finished (const Glib::RefPtr <XmlResult> & result)
{
    /* check for success
     * if it failed, we retry the query in 10 seconds
     */
    if (result->get_status() != 200) {
	Glib::signal_timeout().connect (sigc::ptr_fun (run_query_booting), 10*1000);
	return;
    }

    /* upon success, we can run the second part of init
     */
    PRINT_INFO ("libzix::init_part2 starting\n");
    init_part2 ();
}

void
query_ready_finished (const Glib::RefPtr <XmlResult> & result)
{
    /* check for success
     * if it failed, we retry the query in 10 seconds
     */
    if (result->get_status() != 200) {
	Glib::signal_timeout().connect (sigc::ptr_fun (run_query_ready), 10*1000);
	return;
    }

    PRINT_INFO ("libzix_daemon booting finished\n");
}

void
on_samba_mount_finished(bool mounted)
{
    PRINT_DEBUG("Samba Mounter State: " << (mounted ? "Mounted" : "Not mounted"));

    samba_mount_connection.disconnect();

    init_part3();
}

int init_part2 ()
{
    PRINT_DEBUG ("init_part2: " << __LINE__);

    // instantiate network config early, because
    // its very slow.
    auto net = CNetworkConfig::get_instance ();

    // setup samba mounter
    sambaMounter = SambaMounter::get_instance();
    samba_mount_connection =
        sambaMounter->mount_ready.connect(sigc::ptr_fun(&on_samba_mount_finished));
    sambaMounter->mount();

    return 0;
}

int init_part3 ()
{
    // network, samba and usb are ready -> setup remaining interfaces

    // handle optionally pending procedure/update
    xmlProcessor->init();

    // -> debug interface: TCP localhost@17001
    handlerDebug = SocketInterfaceHandler::create("Unknown", 17001, xmlProcessor);

    // -> COM1: configure only
    configSerialHandler = ConfigSerialInterfaceHandler::create(STR_ZIXINF_COM1, SERIAL_COM1_DEVICE, HANDLER_MASK_COM1_SERIAL, "serial");
    // -> ttyO0: configure only
    configDebugHandler = ConfigSerialInterfaceHandler::create(STR_ZIXINF_DEBUG, SERIAL_DEBUG_DEVICE, HANDLER_MASK_DEBUG_SERIAL, "debug");

    PRINT_DEBUG ("init_part3: " << __LINE__);
    // -> LAN webservice
    handlerWebService = UnixSocketInterfaceHandler::create(
        STR_ZIXINF_LANWEBSERVICE, UNIX_SOCKET_PREFIX "/lan_webservice", xmlProcessor);

    // -> LAN webserver
    handlerWebServer = UnixSocketInterfaceHandler::create(
        STR_ZIXINF_LANWEBSERVER, UNIX_SOCKET_PREFIX "/lan_webserver", xmlProcessor);

    // -> LAN shared folder
    handlerFileLAN = FileInterfaceHandler::create(
        STR_ZIXINF_LANSHAREDFOLDER, xmlProcessor);

    PRINT_DEBUG ("init_part3: " << __LINE__);
    // -> USB folder
    handlerFileUSB = FileInterfaceHandler::create(STR_ZIXINF_USB, xmlProcessor);

    // -> Webservice configuration
    webservice = Webservice::get_instance(  );

    // register all our handlers
    InterfaceHandler::register_handler(handlerDebug);
    InterfaceHandler::register_handler(handlerWebService);
    InterfaceHandler::register_handler(handlerWebServer);
    InterfaceHandler::register_handler(handlerFileLAN);
    InterfaceHandler::register_handler(handlerFileUSB);

    PRINT_DEBUG ("init_part3: " << __LINE__);
    // monitors
    monitorManager = MonitorManager::get_instance();
    monitorManager->addMonitor( Monitor::create("dummy") );
    monitorManager->addMonitor( MonitorCpu::create() );
    monitorManager->addMonitor( MonitorNetwork::create() );
    monitorManager->addMonitor( MonitorMemory::create() );
    monitorManager->addMonitor( MonitorUptime::create(  ) );

    monitorManager->findMonitor( "CPU" )->setAlarmThresholds( 50, 50, eAlarmSlopeTypeRising );

    // create cups config file monitor
    PrinterMonitor::get_instance();

    PRINT_DEBUG ("init_part3: " << __LINE__);

    /* create LanSocketMonitor
     */
    LanSocketMonitor::get_instance ();

    /* ntp monitor
     */
    NtpMonitor::get_instance ();

    PRINT_DEBUG ("init_part3: " << __LINE__);
    // measurement directory has to be monitored, set it up
    try {
        auto conf_handler = ConfHandler::get_instance();
        auto dir = conf_handler->getDirectory("measurements");
        if (!dir.empty()) {
            FileHandler::create_directory(dir);
            auto dir_monitor = MonitorDirectory::create(dir);
            monitorManager->addMonitor( dir_monitor );
            dir_monitor->setAlarmThresholds(DiskUsageManager::get_instance()->number_of_measurements_max());
            dataFreeHandler = DataFreeHandler::create(dir_monitor);
        }
    } catch (const std::exception& ex) {
        PRINT_ERROR("Failed to setup measurements directory watch mechanism: " << ex.what());
    }

    PRINT_DEBUG ("init_part3: " << __LINE__);

    /* disk usage monitoring
     */
    try {
	auto conf_handler = ConfHandler::get_instance();
	auto discUsageHandlerThreshold = conf_handler->getParameter("discUsageHandlerThreshold");
	auto disk_monitor = MonitorDiskUsage::create("/");
	monitorManager->addMonitor( disk_monitor );
	disk_monitor->setAlarmThresholds(std::stoi(discUsageHandlerThreshold));
	dataFreeHandler2 = DataFreeHandler::create(disk_monitor);
	logTruncateHandler = LogTruncateHandler::create(disk_monitor);
    } catch (const std::exception& ex) {
        PRINT_ERROR("Failed to disk usage watch mechanism: " << ex.what());
    }

    PRINT_DEBUG ("init_part3: " << __LINE__);
    // lan watchdogs
    try {
        watchdogManager = WatchdogManager::get_instance();
        watchdogManager->create_lan_watchdogs_by_config();
    } catch (const std::exception& ex) {
        PRINT_ERROR("Error while setting up lan watchdogs: " << ex.what());
    } catch (const Glib::Error& ex) {
        PRINT_ERROR("Error while setting up lan watchdogs: " << ex.what());
    }

    // usb watchdog
    try {
        watchdogManager = WatchdogManager::get_instance();
        watchdogManager->create_usb_watchdog_by_config();
    } catch (const std::exception& ex) {
        PRINT_ERROR("Error while setting up usb watchdogs: " << ex.what());
    } catch (const Glib::Error& ex) {
        PRINT_ERROR("Error while setting up usb watchdogs: " << ex.what());
    }

    PRINT_DEBUG ("init_part3: " << __LINE__);
    // visux configuration
    CNetworkConfig::get_instance ();

#if ! defined (GLOBAL_INSTALLATION)
    std::cout << "not running usb watchdog stuff" << std::endl;
#else
    PRINT_DEBUG ("init_part3: " << __LINE__);
    if(watchdogManager) {
        if(watchdogManager->get_usb_watchdog()) {
            watchdogManager->get_usb_watchdog()->rescan();
	} else {
	    PRINT_DEBUG ("watchdogManager->get_usb_watchdog() is not true");
	}
    } else {
	PRINT_DEBUG ("watchdogManager is not true");
    }
#endif
    PRINT_DEBUG ("init_part3: " << __LINE__);
    run_query_ready ();

    return(0);
}


} // namespace libzix


//---fin.----------------------------------------------------------------------
