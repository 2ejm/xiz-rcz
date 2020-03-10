#ifndef MONITOR_DIRECTORY_H
#define MONITOR_DIRECTORY_H
//-----------------------------------------------------------------------------
///
/// \brief  Monitor for directory
///
///         Monitors amount of files/subdirectories in specific directory
///
/// \date   [20161205] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <string.h>
//#include <giomm/file.h>
#include <glibmm/fileutils.h>

//---Own------------------------------

#include "monitor.h"


//---Declaration---------------------------------------------------------------


using namespace std;
using namespace Glib;

class MonitorDirectory: public Monitor
{
    private:

        Glib::ustring dirName;
        Glib::Dir dir;
        int subdirCount;

    public:
        MonitorDirectory( const Glib::ustring &_dirName );
        static Glib::RefPtr <Monitor>create( const Glib::ustring &_dirName );

        virtual void updateValues();
        virtual Glib::ustring getLogString();

        virtual long getTriggerValue();
};


//-----------------------------------------------------------------------------
#endif // ? ! MONITOR_DIRECTORY_H
