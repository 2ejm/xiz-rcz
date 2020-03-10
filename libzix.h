#ifndef LIBZIX_H
#define LIBZIX_H
//-----------------------------------------------------------------------------
///
/// \brief  Libvisux
///
///         Initializes handlers for the daemon
///         See implementation for further details
///
/// \date   [20161223] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <glibmm/refptr.h>

#include "xml_processor.h"
#include "socket_interface_handler.h"
#include "serial_interface_handler.h"
#include "unix_socket_interface_handler.h"
#include "file_interface_handler.h"
#include "monitor_manager.h"
#include "watchdog_manager.h"
#include "data_free_handler.h"
#include "network_config.h"

//---Own------------------------------


//---Declarations--------------------------------------------------------------

namespace libzix
{
    int init();
}


//---fin.----------------------------------------------------------------------
#endif // ! ? LIBZIX_H
