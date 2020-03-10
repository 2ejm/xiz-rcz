#ifndef MONITOR_UPTIME_H
#define MONITOR_UPTIME_H
//-----------------------------------------------------------------------------
///
/// \brief  Monitor for system uptime
///
///         Reads uptime of system
///
/// \date   [20161206] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <string.h>
#include <fstream>
#include <iomanip>
#include <unistd.h>

//---Own------------------------------

#include "monitor.h"


//---Declaration---------------------------------------------------------------


using namespace std;
using namespace Glib;


class MonitorUptime: public Monitor
{
    private:

        long int total_seconds;
        long int total_hours;
        int days;
        int hours;
        int mins;
        int seks;

    public:
        MonitorUptime( );
        static Glib::RefPtr <Monitor>create(  );

        virtual void updateValues();
        virtual Glib::ustring getLogString();

        virtual long getTriggerValue();
};


//-----------------------------------------------------------------------------
#endif // ? ! MONITOR_UPTIME_H
