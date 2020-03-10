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

#include "monitor_directory.h"


//---Implementation------------------------------------------------------------


MonitorDirectory::MonitorDirectory( const Glib::ustring &_dirName )
    :Monitor( "Directory: " + _dirName)
    ,dirName(_dirName)
    ,dir(dirName)
{
    // Nothing to do here
};


Glib::RefPtr <Monitor>MonitorDirectory::create( const Glib::ustring &_dirName ) /* static */
{
    return ( Glib::RefPtr <Monitor>( new MonitorDirectory(_dirName) ) );
}


void MonitorDirectory::updateValues() /* virtual */
{
    subdirCount=0;

    // "." and ".." are not part of the list;
    // The list is updated automatically; no need to reconstruct the object.
    for (auto&& dirEntry : dir)
    {
        // printf("### File: %s\n", dirEntry.c_str() );
        if( Glib::file_test ( dirName + "/" +dirEntry, Glib::FILE_TEST_IS_DIR) )
            subdirCount++;
    }
    setValid(true);

    return;
}


Glib::ustring MonitorDirectory::getLogString() /* virtual */
{
    Glib::ustring ret=Glib::ustring::compose("Count: %1", subdirCount);

    return(ret);
}


long MonitorDirectory::getTriggerValue() /* virtual */
{
    return(subdirCount);
}


//---fin.----------------------------------------------------------------------
