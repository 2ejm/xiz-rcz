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

#include "monitor_network.h"


//---Implementation------------------------------------------------------------


MonitorNetwork::MonitorNetwork( )
    :Monitor( "Network")
{

};


Glib::RefPtr <Monitor>MonitorNetwork::create(  ) /* static */
{
    return ( Glib::RefPtr <Monitor>( new MonitorNetwork() ) );
}


void MonitorNetwork::updateValues() /* virtual */
{
    std::ifstream stat_stream("/proc/net/dev",ios_base::in);
    string name;
    string dump;
    int dumpInt;
    string unit;

    // Cut of first two header lines;
    std::getline(stat_stream, dump);
    std::getline(stat_stream, dump);

    while( ( ! stat_stream.eof() ) )
    {
        stat_stream >> name >> receive.bytes >> dumpInt >> dumpInt
                >> dumpInt >> dumpInt >> dumpInt >> dumpInt >> dumpInt
                >> transmit.bytes;

        // Could not find a better solution to handle the lines with
        // different layouts (unit is missing in some lines)
        // We just skip the unit here.
        stat_stream.ignore(100, '\n');
        if(name=="eth0:")
        {
            receive.time=time(0);
            transmit.time=time(0);
            break;
        }
        // printf("Name: %s; %ld\n", name.c_str(), value);
    }
    if(name!="eth0:")
    {
        receive.bytes=0;
        receive.time=0;
        transmit.bytes=0;
        transmit.time=0;
    }

    if( prevReceive.time && prevTransmit.time)
    {
        receiveRate=receive.get_rate(prevReceive);
        transmitRate=transmit.get_rate(prevTransmit);
        setValid(true);
    }
    else
    {
        receiveRate=0;
        transmitRate=0;
        setValid(false);
    }

    prevReceive=receive;
    prevTransmit=transmit;
}


Glib::ustring MonitorNetwork::getLogString() /* virtual */
{
    Glib::ustring ret;

    ret=ustring::compose("Receive: %1 Kb/s (%2 Bytes total); Transmit: %3 Kb/s (%4 Bytes total)"
                    , receiveRate,   receive.bytes
                    , transmitRate, transmit.bytes );

    return(ret);
}


long MonitorNetwork::getTriggerValue() /* virtual */
{
    // Network trigger? no better idea than sum of send & received data rate.
    return( receiveRate + transmitRate );
}


//---fin.----------------------------------------------------------------------
