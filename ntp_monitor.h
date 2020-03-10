#ifndef NTP_MONITOR
#define NTP_MONITOR

#include <glibmm/object.h>
#include <glibmm/refptr.h>

#include "state_variable.h"
#include "process_request.h"

/**
 * \brief Monitor ntpd and handle time sync
 */
class NtpMonitor : public Glib::Object
{
    public:
	static Glib::RefPtr <NtpMonitor>  get_instance();

    private:
	static Glib::RefPtr <NtpMonitor> instance;

	static unsigned int nosync_counter;

	NtpMonitor ();
	~NtpMonitor ();

	Glib::RefPtr <ProcessRequest> _ntpstat_proc;
	Glib::RefPtr <ProcessRequest> _ntp_start_proc;

	Glib::RefPtr <Query> _date_time_query;

	StateVariable _ntp_state;

	void time_dctosic (sigc::slot <void, Glib::RefPtr <XmlResult> > callback);
	void time_set_from_result (const Glib::RefPtr <XmlResult> & result);
	void time_sictodc ();
	void on_sictodc_query_finish (const Glib::RefPtr <XmlResult> & result);


	void on_date_time_query_finish (const Glib::RefPtr <XmlResult> & result);
	void on_dctosic_query_finish (const Glib::RefPtr <XmlResult> & result);
	void on_ntp_start_proc_finish (const Glib::RefPtr <ProcessResult> &);
	bool on_monitor_timeout ();
	void on_ntpstat_proc_finish (const Glib::RefPtr <ProcessResult> &);
};
#endif
