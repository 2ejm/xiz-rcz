#ifndef MONITOR_MEMORY_H
#define MONITOR_MEMORY_H
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

#include <string.h>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <glibmm/refptr.h>


//---Own------------------------------


#include "monitor.h"


//---Declaration---------------------------------------------------------------


using namespace std;
using namespace Glib;


class MonitorMemory: public Monitor
{
    private:

        long int total;
        long int used;
        long int free;
        std::map<string, long int> valueMap;

    public:
        MonitorMemory( );
        static Glib::RefPtr <Monitor>create(  );

        virtual void updateValues();
        virtual Glib::ustring getLogString();

        virtual long getTriggerValue();
};


//-----------------------------------------------------------------------------
#endif // ? ! MONITOR_MEMORY_H
