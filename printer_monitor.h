#ifndef _PRINTER_MONITOR_H_
#define _PRINTER_MONITOR_H_
//-----------------------------------------------------------------------------
///
/// \brief  Printer monitor
///
///         see implemention for further details
///
/// \date   [20170607] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <glibmm/object.h>          // Glib::Object
#include <glibmm/refptr.h>
#include <giomm/filemonitor.h>

//---Own--------------------------

#include "process_result.h"
#include "process_request.h"
#include "state_variable.h"


//---Declaration---------------------------------------------------------------


/**
 * ZIX-Printer Handler.
 *
 * This class monitors the cups configuration. On each change the Zix-Conf
 * is updated
 */
class PrinterMonitor : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<PrinterMonitor>;

    static inline RefPtr get_instance()
    {
        if (!instance)
            instance = RefPtr(new PrinterMonitor());
        return instance;
    }

private:
    static RefPtr instance;
    Glib::RefPtr<ProcessRequest> _process;
    StateVariable _printer_state;

    bool on_cups_config_check_timeout();
    void handle_finished( const Glib::RefPtr<ProcessResult> &result);

    PrinterMonitor();
};


//---fin.----------------------------------------------------------------------
#endif /* _PRINTER_MONITOR_H_ */
