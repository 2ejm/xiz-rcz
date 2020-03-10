
#include "ntp_monitor.h"
#include "xml_conf_parameter.h"
#include "xml_query_get_parameter.h"
#include "xml_query_set_datetime.h"

#include "conf_handler.h"
#include "network_config.h"

Glib::RefPtr <NtpMonitor> NtpMonitor::instance;

unsigned int NtpMonitor::nosync_counter = 0;

Glib::RefPtr <NtpMonitor>
NtpMonitor::get_instance()
{
    if (!instance)
	instance = Glib::RefPtr <NtpMonitor>(new NtpMonitor());
    return instance;
}

NtpMonitor::NtpMonitor ()
    : _ntp_state ("statusLanTime")
{
    /* Before starting ntp, when fetch the time from DC
     */
    time_dctosic (sigc::mem_fun (*this, &NtpMonitor::on_date_time_query_finish));
}

void
NtpMonitor::on_date_time_query_finish (const Glib::RefPtr <XmlResult> & result)
{
    PRINT_DEBUG ("NtpMonitor::on_date_time_query_finish");

    if (result->get_status () != 200) {
	PRINT_ERROR ("Failed to get date/time from DC status: " << result->get_status ());
	PRINT_ERROR ("NtpMonitor aborts");
	return;
    }

    /* call helper, to extract the time from
     * result, and set sic clock
     */
    time_set_from_result (result);

    /* now we can start ntpd
     *
     * TODO: maybe validate, whether we have really setup the date above ?
     *       although, we can probably assume, this happened.
     */

    _ntp_start_proc = ProcessRequest::create ({ "/etc/init.d/ntp", "start" },
					      ProcessRequest::DEFAULT_TIMEOUT,
					      false,
					      "",
					      false);

    _ntp_start_proc->finished.connect (sigc::mem_fun (*this, &NtpMonitor::on_ntp_start_proc_finish));
    _ntp_start_proc->start_process();
}

void
NtpMonitor::time_dctosic (sigc::slot <void, Glib::RefPtr <XmlResult> > callback)
{
    std::vector <Glib::RefPtr <XmlRestrictionParameter> > date_time {
	XmlRestrictionParameter::create ("date"),
	XmlRestrictionParameter::create ("time")
    };

    auto dc = QueryClient::get_instance ();
    auto xq = XmlQueryGetParameter::create (date_time);

    _date_time_query = dc->create_query (xq);
    _date_time_query->finished.connect (callback);
    dc->execute(_date_time_query);
}

void
NtpMonitor::time_sictodc ()
{
    Glib::DateTime date=Glib::DateTime::create_now_utc( );
    auto dc = QueryClient::get_instance ();
    auto xq = XmlQuerySetDateTime::create (date.format ("%F"), date.format ("%T"));

    _date_time_query = dc->create_query (xq);
    _date_time_query->finished.connect (sigc::mem_fun (*this, &NtpMonitor::on_sictodc_query_finish));
    dc->execute(_date_time_query);
}

void
NtpMonitor::on_sictodc_query_finish (const Glib::RefPtr <XmlResult> & result)
{
    if (result->get_status () != 200) {
	PRINT_ERROR ("Failed to set date/time status: " << result->get_status ());
	return;
    }

    PRINT_DEBUG ("Successfully set DC datetime");
}

void NtpMonitor::time_set_from_result (const Glib::RefPtr <XmlResult> & result)
{
    auto conf = ConfHandler::get_instance ();

    /* evaluate result and set the SIC local clock
     */
    for (auto p : result->getParameterList ().get_all <XmlConfParameter> ("parameter")) {

	/* first check, whether the dynamic casts
	 * is ok.
	 */
	if (!p)
	    continue;

	/* now check what we have here
	 */
	if (p->get_id () == "time") {
	    PRINT_DEBUG ("got time " << p->get_value ().raw ());
	    conf->setConfParameter ("time", "", p->get_value ());
	} else if (p->get_id () == "date") {
	    PRINT_DEBUG ("got date " << p->get_value ().raw ());
	    conf->setConfParameter ("date", "", p->get_value ());
	}
    }
}

void
NtpMonitor::on_dctosic_query_finish (const Glib::RefPtr <XmlResult> & result)
{
    if (result->get_status () != 200) {
	PRINT_ERROR ("Failed to get date/time from DC status: " << result->get_status ());
	return;
    }

    /* call helper, to extract the time from
     * result, and set sic clock
     */
    time_set_from_result (result);
}


void
NtpMonitor::on_ntp_start_proc_finish (const Glib::RefPtr <ProcessResult> & result)
{
    if (!result->success ()) {
	PRINT_ERROR ("failed to start ntpd... continuing anyways");
    }

    /* we still want immediate feedback here
     */
    on_monitor_timeout ();

    Glib::signal_timeout ().connect (sigc::mem_fun(*this, &NtpMonitor::on_monitor_timeout), 60*1000);
}


NtpMonitor::~NtpMonitor ()
{ }

void
NtpMonitor::on_ntpstat_proc_finish (const Glib::RefPtr <ProcessResult> & result)
{
    /* ntpstat returns.
     * We can now evaluate the result
     */

    switch (result->get_child_status ()) {
	case 0:
	    _ntp_state.set_state ("synchronized");
	    break;
	case 1:
	    _ntp_state.set_state ("unsynchronized");
	    break;

	case 2:
	    _ntp_state.set_state ("notconnected");
	    break;

	default:
	    PRINT_ERROR ("NtpMonitor::on_ping_process_finish (): ntpstat returned unknown status: " << result->get_child_status ());
	    _ntp_state.set_state ("notconnected");
	    break;
    }
}

bool
NtpMonitor::on_monitor_timeout ()
{
    /* first check, for network online
     */
    auto net = CNetworkConfig::get_instance ();

    if (!net->online ()) {
	/* not online -> "nodevice"
	 * dc to sic
	 */
	_ntp_state.set_state ("nodevice");
	return true;
    }

    /* start ntpstat process
     */
    _ntpstat_proc = ProcessRequest::create ({ "ntpstat" },
					 ProcessRequest::DEFAULT_TIMEOUT,
					 false,
					 "",
					 false);

    _ntpstat_proc->finished.connect (sigc::mem_fun (*this, &NtpMonitor::on_ntpstat_proc_finish));
    _ntpstat_proc->start_process();

    return true;
}
