//-----------------------------------------------------------------------------
///
/// \brief  Printer monitor
///
///         Monitors cups configuration file to check when printer config was
///         updated.
///         On change all printers get removed and printers via "lpstat -p" are
///         added.
///
/// \date   [20170607] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <giomm/filemonitor.h>
#include <giomm/file.h>
#include <regex>


//---Own------------------------------

#include "log.h"
#include "printer_monitor.h"
#include "process_request.h"
#include "utils.h"
#include "conf_handler.h"        // updatePrinters()
#include "network_config.h"


//---Implementation------------------------------------------------------------


PrinterMonitor::RefPtr PrinterMonitor::instance;

PrinterMonitor::PrinterMonitor()
    : _printer_state ("statusLanPrinter")
{
   lDebug("Initialize cups config monitor");

   /* run callback immediately, before starting timeout
    */
   on_cups_config_check_timeout ();

   Glib::signal_timeout().connect(
	   sigc::mem_fun(*this, &PrinterMonitor::on_cups_config_check_timeout), 10*1000);
}


bool
PrinterMonitor::on_cups_config_check_timeout()
{
    auto net = CNetworkConfig::get_instance ();

    if (!net->online ()) {
	_printer_state.set_state ("nodevice");
	return true;
    }

   _process = ProcessRequest::create(std::vector<std::string>{"lpstat", "-p"}, 0, true);
   _process->finished.connect( sigc::mem_fun( *this, &PrinterMonitor::handle_finished) );
   _process->set_no_log_error ();
   _process->start_process();

   return true;
}


void
PrinterMonitor::handle_finished( const Glib::RefPtr<ProcessResult> &result)
{
    _process.reset ();

    std::stringstream ss(result->stdout());
    std::string str;

    lDebug("lpstat output: %s\n", result->stdout().c_str());

    std::regex rgx("printer (.+) is (.+)\\..*");
    std::regex rgp("printer (.+) now printing (.+)\\..*");
    std::smatch match;
    std::list<std::string> printers;

    Glib::RefPtr<ConfHandler> conf=ConfHandler::get_instance();
    Glib::ustring lan_printer_name = conf->getConfParameter ("lanPrinterName");
    std::string lan_printer_state;

    while(std::getline(ss,str,'\n'))
    {
        if (std::regex_match(str, match, rgx))
        {
            printers.push_back(match[1]);
            PRINT_DEBUG( "Printer : " << match[1]);

	    if (match[1] == std::string (lan_printer_name)) {
		/* if we match lan_printer_name, save its status
		 */
		lan_printer_state = match[2];
	    }
        } else if (std::regex_match (str, match, rgp)) {
            printers.push_back(match[1]);
            PRINT_DEBUG( "Printer : " << match[1]);

	    if (match[1] == std::string (lan_printer_name)) {
		/* rgp matches somthing like:
		 * printer Deskjet-2050-J510-series now printing Deskjet-2050-J510-series-668.
		 *
		 * match[2] is the jobname, but we are not interested in that.
		 */
		lan_printer_state = "busy";
	    }
	}
    }

    try
    {
        conf->updatePrinters(printers);

    }
    catch (const std::exception &ex)
    {
        lError( ex.what() );
    }
    catch ( ... )
    {
        lError( "Could not update printers" );
    }

    if (lan_printer_state.empty ()) {
	_printer_state.set_state ("nodevice");
    } else if (lan_printer_state == "busy") {
	_printer_state.set_state ("busy");
    } else if (lan_printer_state == "idle") {
	_printer_state.set_state ("idle");
    } else {
	/* we dont seem to recognize this status,
	 * lets consider this an error for now.
	 */
	_printer_state.set_state ("error");
    }

    /* and reset _process
     */
    _process.reset ();

    return;
}


//---fin.----------------------------------------------------------------------
