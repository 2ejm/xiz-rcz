//-----------------------------------------------------------------------------
///
/// \brief  Logging functions
///
///         Control log output via log level.
///         Unfortunately we have to use the libzix way for logging.
///         The loging mechanism can be switched.
///         The high/veryhigh debug makros "lHighDebug/lVHDebug" always use
///         direct debugging;
///
/// \date   [20161205] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <glib/gprintf.h>

#include <glibmm/refptr.h>

//---Own------------------------------

#include "log.h"
#include "log_handler.h"
#include <iostream>              // cout


//---Implementation------------------------------------------------------------


ELogLevel eLogLevel=ELogLevelDebug;
ELogMechanism eLogMechanism=ELogMechanismDirect;

void setInternLogLevel( ELogLevel _eLogLevel )
{
    eLogLevel=_eLogLevel;
}

void setLogMechanism( ELogMechanism _eLogMechanism )
{
   eLogMechanism=_eLogMechanism;
}

const char *logLevelStringMap[]=
{
   [ELogLevelFatalExit] = "Fatal Error",
   [ELogLevelFatal] = "Fatal Error",
   [ELogLevelError] = "Error",
   [ELogLevelWarn] = "Warning",
   [ELogLevelInfo] = "Info",
   [ELogLevelHint] = "Info",
   [ELogLevelDebug] = "Debug",
   [ELogLevelHighDebug] = "Debug",
   [ELogLevelVeryHighDebug] = "Debug",
   [ELogLevelLast] = "-----",
};

void loggerLibzix( ELogLevel _eLogLevel, const char *format, va_list argp)
{
   if (_eLogLevel > ELogLevelDebug) {
       /* libzix can not cope with debug Level higher than debug
	*/
       return;
   }
   char *str=g_strdup_vprintf (format, argp);
   Glib::RefPtr <LogHandler> logHandler=LogHandler::get_instance();

   logHandler->log_internal (logLevelStringMap[_eLogLevel], "Audit", str);

   free (str);

   if (_eLogLevel <= ELogLevelFatalExit)
      exit(22);
}

void loggerDirect( ELogLevel _eLogLevel, const char *format, va_list argp)
{
   if( _eLogLevel <= eLogLevel )
   {
      if( _eLogLevel <= ELogLevelError )
         vfprintf( stderr, format, argp);
      else
         vprintf(format, argp);
   }

   if( _eLogLevel <= ELogLevelFatalExit )
      exit(22);

   return;
}

void loggerLibzix( const Glib::ustring &level, const Glib::ustring &str)
{
   Glib::RefPtr <LogHandler> logHandler=LogHandler::get_instance();
   logHandler->log_internal(level, "Audit",
                            str );

   return;
}

void loggerDirect( const Glib::ustring &level, const Glib::ustring &str)
{
   std::cout << "[" << level << "]: " << str << std::endl;

   return;
}

void logger( ELogLevel _eLogLevel, const char *format, ...)
{
   va_list argp;
   va_start(argp, format);

   switch(eLogMechanism)
   {
      case ELogMechanismLibzix:
         loggerLibzix( _eLogLevel, format, argp);
         break;
      default:
         loggerDirect( _eLogLevel, format, argp);
         break;
   }
   va_end(argp);
   return;
}

void logger( const Glib::ustring &level, const Glib::ustring &str)
{
   switch(eLogMechanism)
   {
      case ELogMechanismLibzix:
         loggerLibzix( level, str);
         break;
      default:
         loggerDirect( level, str);
         break;
   }
   return;
}

bool doInternLog( ELogLevel _eLogLevel )
{
    if( _eLogLevel <= eLogLevel )
        return(true);

    return(false);
}

//---fin.----------------------------------------------------------------------
