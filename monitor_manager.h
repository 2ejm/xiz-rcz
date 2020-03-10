#ifndef MONITOR_MANAGER_H
#define MONITOR_MANAGER_H
//-----------------------------------------------------------------------------
///
/// \brief  Monitor manager
///
///         Hosts monitors.
///
///         See implementation for further information
///
/// \date   [20161205] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <string>
#include <glibmm/ustring.h>
#include <glibmm/object.h>
#include <glibmm/main.h>            // SignalTimeout
//#include <queue>                    // SignalTimeout
#include <giomm/file.h>

//---Own------------------------------

#include "monitor.h"
#include "log_handler.h"


//---Declaration---------------------------------------------------------------


class MonitorManager: public Glib::Object
                    //, public Glib::SignalTimeout
{
    private:
        MonitorManager();

        std::vector < Glib::RefPtr <Monitor> > monitors;

        Glib::RefPtr<Gio::File> _file;
        Glib::RefPtr<Gio::FileOutputStream> _stream;
        std::string _log_file;
        LogHandler::LogLevel originLogLevel;
        int alarmCounter;

        static Glib::RefPtr<MonitorManager> instance;

    public:

        static inline Glib::RefPtr<MonitorManager> get_instance()
        {
            if (!instance)
                instance = Glib::RefPtr<MonitorManager>(new MonitorManager());
            return instance;
        }

        void addMonitor( Glib::RefPtr <Monitor> monitor );
        bool timeout_handler();
        Glib::RefPtr <Monitor> findMonitor( const Glib::ustring &name);
        void slotAlarm( EAlarmDirection eAlarmDirection, Monitor &monitor);
        void log(const Glib::ustring &message);
        void log_truncate(int number_of_log_keep);
};


//---fin-----------------------------------------------------------------------
#endif // ? ! MONITOR_MANAGER_H
