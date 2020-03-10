#ifndef MONITOR_NETWORK_H
#define MONITOR_NETWORK_H
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

#include <string.h>
#include <fstream>
#include <iomanip>
#include <unistd.h>

//---Own------------------------------

#include "monitor.h"


//---Declaration---------------------------------------------------------------


using namespace std;
using namespace Glib;


// Use class instead of struct in order to have an copy functionality.
class NetStat
{
    public:
        long int bytes;
        time_t time;

        /// brief calculates transfer rate in Kb/s
        int get_rate( NetStat &prev)
        {
            long int byteDiff=bytes-prev.bytes;
            time_t timeDiff=time-prev.time;

            if(timeDiff>0)
                return( ( byteDiff / timeDiff) / 1024 );

            return(0);
        }

};


class MonitorNetwork: public Monitor
{
    private:

        NetStat transmit, prevTransmit;
        NetStat receive, prevReceive;

        long int transmitRate;
        long int receiveRate;

    public:
        MonitorNetwork( );
        static Glib::RefPtr <Monitor>create(  );

        virtual void updateValues();
        virtual Glib::ustring getLogString();

        virtual long getTriggerValue();
};


//-----------------------------------------------------------------------------
#endif // ? ! MONITOR_MEMORY_H
