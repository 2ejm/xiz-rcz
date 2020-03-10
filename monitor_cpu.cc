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

#include <glibmm/refptr.h>

//---Own------------------------------

#include "monitor_cpu.h"


//---Implementation------------------------------------------------------------


MonitorCpu::MonitorCpu()
    :Monitor( "CPU" )
{
    firstrun=true;
};


Glib::RefPtr <Monitor>MonitorCpu::create(  ) /* static */
{
    return ( Glib::RefPtr <Monitor>( new MonitorCpu() ) );
}


void MonitorCpu::updateValues() /*virtual*/
{
    std::ifstream stat_stream("/proc/stat",ios_base::in);
    string cpu;
    stat_stream >> cpu >>  cpuStat.user >> cpuStat.nice >> cpuStat.system >> cpuStat.idle;

    if( ! firstrun )
    {
        // Confusing magic reloved:
        //      time difference when done something divided by
        //      total time difference.
        //      E.g. 100ms/100ms -> 100% usage
        loadavg = (long double)( cpuStat.usageSum() - prCpuStat.usageSum() )
                / (long double)( cpuStat.totalSum() - prCpuStat.totalSum() );
        setValid(true);
    }
    else
    {
        loadavg=0;
        firstrun=false;
    }

    prCpuStat=cpuStat;

    return;
}


Glib::ustring MonitorCpu::getLogString() /* virtual */
{
    Glib::ustring ret;

    ret=ustring::compose("The current CPU utilization is : %1"
                        ,ustring::format(std::fixed, std::setprecision(1), loadavg * 100) );

    return(ret);
}


long MonitorCpu::getTriggerValue() /* virtual */
{
    return( loadavg * 100 );
}


//---fin.----------------------------------------------------------------------
