//-----------------------------------------------------------------------------
///
/// \brief  Monitor for memory
///
///         Measures memory usage.
///
/// \date   [20161205] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------


//---Own------------------------------

#include "monitor_memory.h"


//---Implementation------------------------------------------------------------


MonitorMemory::MonitorMemory( )
    :Monitor( "Memory")
{
    total=used=free=0;
};


Glib::RefPtr <Monitor>MonitorMemory::create(  ) /* static */
{
    return ( Glib::RefPtr <Monitor>( new MonitorMemory() ) );
}


void MonitorMemory::updateValues() /* virtual */
{
    std::ifstream stat_stream("/proc/meminfo",ios_base::in);
    string name;
    long int value;
    string unit;

    while( ( ! stat_stream.eof() ) )
    {
        stat_stream >> name >> value;

        // Could not find a better solution to handle the lines with
        // different layouts (unit is missing in some lines)
        // We just skip the unit here.
        stat_stream.ignore(100, '\n');
        valueMap[name]=value;
        // printf("Name: %s; %ld\n", name.c_str(), value);
    }
    total=valueMap["MemTotal:"];
    free=valueMap["MemFree:"];
    used=total-free;
    setValid(true);
}


Glib::ustring MonitorMemory::getLogString() /* virtual */
{
    Glib::ustring ret;

    ret=ustring::compose("Total: %1 kB; Used: %2 kB; Free:%3 kB"
                        ,total, used, free);

    return(ret);
}


long MonitorMemory::getTriggerValue() /* virtual */
{
    return(free);
}


//---fin.----------------------------------------------------------------------
