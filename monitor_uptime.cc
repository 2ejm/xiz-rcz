//-----------------------------------------------------------------------------
///
/// \brief  Monitor for network
///
///         Measures network throughput.
///
/// \date   [20161205] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <glibmm/refptr.h>

//---Own------------------------------

#include "monitor_uptime.h"


//---Implementation------------------------------------------------------------


MonitorUptime::MonitorUptime(  )
    :Monitor( "Uptime")
{

};


Glib::RefPtr <Monitor>MonitorUptime::create(  ) /* static */
{
    return ( Glib::RefPtr <Monitor>( new MonitorUptime() ) );
}


void MonitorUptime::updateValues() /* virtual */
{
    std::ifstream stat_stream("/proc/uptime",ios_base::in);
    stat_stream >> total_seconds;

    total_hours = ( (int)( total_seconds / 60) / 60 ) ;

    days =( (int)( total_seconds / 60) / 60 ) / 24;
    hours=( (int)( total_seconds / 60) / 60 ) % 24;
    mins= ( (int)( total_seconds / 60)      ) % 60;
    seks= ( (int)( total_seconds     )      ) % 60;
    setValid(true);

    return;
}


Glib::ustring MonitorUptime::getLogString() /* virtual */
{
    Glib::ustring ret;

    ret=Glib::ustring::compose(
                "%1 days, %2:%3:%4"
                , days
                , hours
                ,ustring::format(std::setfill(L'0'), std::setw(2), mins)
                ,ustring::format(std::setfill(L'0'), std::setw(2), seks)
                );

    return(ret);
}


long MonitorUptime::getTriggerValue() /* virtual */
{
    return( total_hours );
}


//---fin.----------------------------------------------------------------------
