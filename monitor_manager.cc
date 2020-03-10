//-----------------------------------------------------------------------------
///
/// \brief  Monitor manager
///
///         Hosts monitors.
///         If an monitor sends an alarm the current log level has to be stored
///         and rised to "debug"-level.
///         When the alarm disappears the origin log level has to be restored.
///
/// \date   [20161205] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

//---Own------------------------------

#include "monitor_manager.h"
#include "conf_handler.h"
#include "log_handler.h"
#include "file_handler.h"
#include "time_utilities.h"
#include "log.h"


//---Implementation------------------------------------------------------------

Glib::RefPtr<MonitorManager> MonitorManager::instance;

MonitorManager::MonitorManager()
        //:Glib::SignalTimeout()
        :originLogLevel( LogHandler::LogLevel::INVALID )
        ,alarmCounter(0)
{
    #ifdef GLOBAL_INSTALLATION
        _log_file="/var/log/monitor.log";
    #else
        _log_file="monitor.log";
    #endif

    try {
        //Glib::ustring log_file=ConfHandler::get_instance()->getDirectory( "" );
        _file = Gio::File::create_for_path(_log_file);
        _stream = _file->append_to();
    } catch (const Glib::Error& ex) {
        throw std::logic_error(
            Glib::ustring::compose(
                "Failed to open monitor log file '%1': '%2'", _log_file, ex.what()));
    }

    Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &MonitorManager::timeout_handler), 100000);

}

void MonitorManager::addMonitor( Glib::RefPtr <Monitor> monitor )
{
    monitors.push_back( monitor );
    monitor->alarm.connect(
        sigc::mem_fun(*this, &MonitorManager::slotAlarm ));
}


void MonitorManager::log(const Glib::ustring &message)
{
    gsize bytes_written;
    _stream->write_all( Glib::ustring::compose( "%1 %2\n"
             ,TimeUtilities::get_timestamp()
             ,message )
             , bytes_written );
    return;
}

void MonitorManager::log_truncate(int number_of_log_keep)
{
    _stream->close();

    FileHandler::log_truncate(_log_file, number_of_log_keep);

    /* Gets an output stream for appendig data to the file. If
     * the file doesn't exist it is created.
     */
    _stream = _file->append_to();
}

bool MonitorManager::timeout_handler()
{

    lDebug("Monitor manager timer...\n");
    for (auto monitor : monitors)
    {
        monitor->updateValues();

        log( Glib::ustring::compose( "%1: %2"
                 ,monitor->getName()
                 ,monitor->getLogString() ) );
        lDebug("   %s: %s\n", monitor->getName().c_str(),
                    monitor->getLogString().c_str() );
        monitor->evaluateAlarms();

        #ifdef DEBUG_CLASSES
            //monitor->debugInfo();
        #endif
    }

    return(true);
}


Glib::RefPtr <Monitor> MonitorManager::findMonitor( const Glib::ustring &name)
{
    for (auto monitor : monitors)
    {
        if( monitor->getName() == name )
            return(monitor);
    }
    return( Glib::RefPtr <Monitor>( nullptr ) );
}


void MonitorManager::slotAlarm( EAlarmDirection eAlarmDirection, Monitor &monitor)
{
    Glib::RefPtr <LogHandler> logHandler=LogHandler::get_instance();

    lWarn("### GOT Alarm!!! monitor: %s; direction: %s; triggerValue: %d; appearTriggerValue: %d\n"
           , monitor.getName().c_str(), ( eAlarmDirection == eAlarmDirectionAppear ) ? "Appear" : "Disappear"
           , monitor.getTriggerValue()
           , monitor.getAppearTriggerValue()
           );

    if( eAlarmDirection == eAlarmDirectionAppear )
        alarmCounter++;
    else
        alarmCounter--;

    if( ( eAlarmDirection == eAlarmDirectionAppear ) && ( alarmCounter == 1 ) )
    {
        originLogLevel=logHandler->get_log_level();
        logHandler->set_log_level( LogHandler::LogLevel::DEBUG );
        lWarn("###    raised log level\n");
    }
    else if ( alarmCounter == 0 )
    {
        // Was the log level even stored before?
        if( originLogLevel != LogHandler::LogLevel::INVALID )
        {
            logHandler->set_log_level( originLogLevel );
            lWarn("###    restored log level (%d)\n", originLogLevel);
        }
    }

    log( Glib::ustring::compose( "Alarm %1 in monitor %2"
         , (( eAlarmDirection == eAlarmDirectionAppear )?"appeared":"disappeared")
         , monitor.getName() ) );

    logHandler->log_internal("Error", "monitor",
                             std::string("Alarm ")
                                         + (( eAlarmDirection == eAlarmDirectionAppear )?"appeared":"disappeared")
                                         + " in monitor " + monitor.getName() );
    logHandler->flush();

    return;
}


//---fin.----------------------------------------------------------------------
