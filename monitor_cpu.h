#ifndef MONITOR_CPU_H
#define MONITOR_CPU_H
//-----------------------------------------------------------------------------
///
/// \brief  Monitor for cpu
///
///         Measures cpu usage.
///
/// \date   [20161205] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <string.h>
#include <fstream>
#include <iomanip>


//---Own------------------------------

#include "monitor.h"


//---Declaration---------------------------------------------------------------


using namespace std;
using namespace Glib;

// Use class instead of struct in order to have an copy functionality.
class CpuStat
{
    public:
        int user;
        int nice;
        int system;
        int idle;

        int totalSum()
        {
            return(
               user + nice + system + idle );
        }

        int usageSum()
        {
            return(
               user + nice + system );
        }

};


class MonitorCpu: public Monitor
{
    private:

    CpuStat cpuStat, prCpuStat;
    long double loadavg;
    bool firstrun;

    public:
        MonitorCpu();
        static Glib::RefPtr <Monitor>create(  );

        virtual void updateValues();
        virtual Glib::ustring getLogString();

        virtual long getTriggerValue();
};


//-----------------------------------------------------------------------------
#endif // ? ! MONITOR_CPU_H
