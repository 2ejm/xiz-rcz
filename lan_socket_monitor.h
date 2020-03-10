#ifndef LAN_SOCKET_MONITOR
#define LAN_SOCKET_MONITOR

#include <glibmm/object.h>
#include <glibmm/refptr.h>

#include "state_variable.h"
#include "process_request.h"
/**
 * \brief Monitor Network by pinging some host/ip
 */
class LanSocketMonitor : public Glib::Object
{
    public:
	static Glib::RefPtr <LanSocketMonitor>  get_instance();

    private:
	static Glib::RefPtr <LanSocketMonitor> instance;

	LanSocketMonitor ();
	~LanSocketMonitor ();

	Glib::RefPtr <ProcessRequest> _ping_proc;
	StateVariable _lan_socket_state;

	bool on_monitor_timeout ();
	void on_ping_process_finish (const Glib::RefPtr <ProcessResult> &);
};
#endif
