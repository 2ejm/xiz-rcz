#ifndef LOG_H
#define LOG_H
//-----------------------------------------------------------------------------
///
/// \brief  Just Logging
///
///         See implementation for further details
///
/// \date   [20161222] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <stdio.h>
#include <string>
#include <list>
#include <glibmm/ustring.h>
#include <glibmm/object.h>
#include <glibmm/bytes.h>
#include <queue>
#include <giomm/file.h>

//---Own------------------------------


//---Declaration---------------------------------------------------------------


enum ELogLevel
{
    ELogLevelFatalExit      = 0,
    ELogLevelFatal          = 1,
    ELogLevelError          = 2,
    ELogLevelWarn           = 3,
    ELogLevelInfo           = 4,
    ELogLevelHint           = 5,
    ELogLevelDebug          = 6,
    ELogLevelHighDebug      = 7,
    ELogLevelVeryHighDebug  = 8,

    ELogLevelLast           = 9,    // Just a enum marker
};

enum ELogMechanism
{
    ELogMechanismDirect      = 0,
    ELogMechanismLibzix      = 1,
};

void logger( ELogLevel _eLogLevel, const char *format, ...);
void setLogMechanism( ELogMechanism _eLogMechanism );
void loggerDirect( ELogLevel _eLogLevel, const char *format, va_list argp);
void logger( const Glib::ustring &logLevel, const Glib::ustring &str);
bool doInternLog( ELogLevel _eLogLevel );

#define lFatalExit( format...) logger( ELogLevelFatalExit, format )
#define lFatal( format...) logger( ELogLevelFatal, format )
#define lError( format...) logger( ELogLevelError, format )
#define lWarn( format...) logger( ELogLevelWarn, format )
#define lHint( format...) logger( ELogLevelHint, format )
#define lInfo( format...) logger( ELogLevelInfo, format )
#define lDebug( format...)  logger( ELogLevelDebug, format )
#define lHighDebug( format...)  logger( ELogLevelHighDebug, format )
#define lVHDebug( format...)  logger( ELogLevelVeryHighDebug, format )

void setInternLogLevel( ELogLevel logLevel );


//-----------------------------------------------------------------------------
#endif // ? ! LOG_H
