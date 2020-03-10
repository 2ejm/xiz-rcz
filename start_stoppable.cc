#include "conf_handler.h"
#include "utils.h"

#include "start_stoppable.h"

StartStoppable::StartStoppable(ZixInterface inf, bool use_conf_signal) :
    _inf{inf}, _par_id{inf.get_config_par()}
{
    // start/stop not for empty or serial; serial handles start/stop alone
    if (_par_id.empty() || _par_id == "serial")
        return;

    auto conf_handler = ConfHandler::get_instance();
    _current_state    = conf_handler->getParameter(_par_id);

    if (_current_state.empty())
        EXCEPTION("Failed to get current state for parameter " << _par_id);

    if (use_conf_signal) {
	conf_handler->confChangeAnnounce.connect(
	    sigc::mem_fun(*this, &StartStoppable::on_config_changed));

	if (_current_state == "enabled") {
	    start ();
	}
    }
}

void StartStoppable::process(const std::string& command)
{
    if (_proc)
        return;
    if (command != "start" && command != "stop" && command != "restart")
        return;

    std::string process{""};
    if (_inf == ZixInterface::Inf::LANwebserver)
        process = "lighttpd";
    if (_inf == ZixInterface::Inf::LANwebservice)
        process = "lighttpd_ws";

    if (process.empty())
        return;

    try {
        _proc = ProcessRequest::create(
            {
#if defined ( GLOBAL_INSTALLATION )
                "/etc/init.d/" + process,
#else
                "/bin/echo", process,
#endif
                command }, ProcessRequest::DEFAULT_TIMEOUT);
        _proc->finished.connect(
            sigc::mem_fun(*this, &StartStoppable::on_proc_finish));
        _proc->start_process();
    } catch (const std::exception& ex) {
        _proc.reset();
        PRINT_ERROR("Failed to " << command << " " << process);
    }
}

void StartStoppable::start()
{
    process("start");
}

void StartStoppable::restart()
{
    process("restart");
}

void StartStoppable::stop()
{
    process("stop");
}

void StartStoppable::on_config_changed(
    const Glib::ustring& par_id, const Glib::ustring& value, int &handlerMask)
{
    (void) handlerMask;
    (void) value;

    if ((par_id != "all") && (par_id != _par_id)) {
	/* our parameter did not change, we just return
	 */
	return;
    }

    /* read out the parameter
     */
    auto conf_handler = ConfHandler::get_instance();
    auto param_value  = conf_handler->getParameter(_par_id);

    /* check, whether parameter is actually changed
     */
    if (_current_state == param_value)
        return;

    /* ok... param is changed, now we need to start or stop
     * accordingly, and then save the _current_state
     */
    if (param_value == "disabled")
        stop();
    if (param_value == "enabled")
        start();
    _current_state = param_value;
}


void StartStoppable::on_proc_finish(const Glib::RefPtr<ProcessResult>& result)
{
    if (!result->success())
        PRINT_ERROR("Start/Stop process for interface " << _inf.to_string() << " failed.");
    _proc.reset();
}
