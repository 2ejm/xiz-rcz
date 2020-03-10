
#include "lan_socket_monitor.h"

#include "conf_handler.h"
#include "network_config.h"

Glib::RefPtr <LanSocketMonitor> LanSocketMonitor::instance;

Glib::RefPtr <LanSocketMonitor>
LanSocketMonitor::get_instance()
{
    if (!instance)
	instance = Glib::RefPtr <LanSocketMonitor>(new LanSocketMonitor());
    return instance;
}

LanSocketMonitor::LanSocketMonitor ()
    : _lan_socket_state ("statusLanSocket")
{
    /* first call the callback immediately, and then add the timeout
     */
    on_monitor_timeout ();

    Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &LanSocketMonitor::on_monitor_timeout), 60*1000);
}

LanSocketMonitor::~LanSocketMonitor ()
{ }

void
LanSocketMonitor::on_ping_process_finish (const Glib::RefPtr <ProcessResult> & result)
{
    /* ping returns.
     * We can now evaluate the result
     */

    if (result->success ()) {
	_lan_socket_state.set_state ("connected");
    } else {
	_lan_socket_state.set_state ("notconnected");
    }
}

bool
LanSocketMonitor::on_monitor_timeout ()
{
    /* XXX: need to check network up/down
     *      so that we could return nodevice
     */

    auto conf = ConfHandler::get_instance ();
    auto net  = CNetworkConfig::get_instance ();

    Glib::ustring lan_socket_peer = conf->getConfParameter ("lanSocketPeer");

    if (!net->online ()) {
	_lan_socket_state.set_state ("nodevice");
	return true;
    }

    if (lan_socket_peer.empty ()) {
	/* no peer configured
	 * try again later
	 */
	_lan_socket_state.set_state ("notconnected");
	return true;
    }

    /* check whether lan socket peer contains ":"
     * and strip everything from the ":" on
     *
     * this makes 192.168.0.1 from 192.168.0.1:234
     */
    Glib::ustring::size_type pos = lan_socket_peer.find (":");

    if (pos != Glib::ustring::npos) {
	lan_socket_peer = lan_socket_peer.substr (0, pos-1);
    }

    /* now start the ping process
     */
    _ping_proc = ProcessRequest::create ({ "ping", "-c", "1", lan_socket_peer },
					 ProcessRequest::DEFAULT_TIMEOUT,
					 false,
					 "",
					 false);

    _ping_proc->finished.connect (sigc::mem_fun (*this, &LanSocketMonitor::on_ping_process_finish));
    _ping_proc->start_process();

    return true;
}
