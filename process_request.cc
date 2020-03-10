#include <stdexcept>

#include <sys/types.h>
#include <signal.h>

#include <glibmm/stringutils.h>

#include "process_request.h"
#include "log_handler.h"
#include "time_utilities.h"
#include "std_utils.h"
#include "utils.h"

ProcessRequest::ProcessRequest(
    const std::vector<std::string>& argv, unsigned timeout, bool capture_stdout,
    const std::string& stdout_file, bool capture_stderr)
  : _argv{argv}
  , _stdout_file{stdout_file}
  , _timeouted{false}
  , _capture_stdout{capture_stdout}
  , _capture_stderr{capture_stderr}
  , _timeout{timeout}
  , _no_log_error (false)

{ }

void ProcessRequest::start_process()
{
    /*
     * Default flags:
     *  - SPAWN_SEARCH_PATH -> Use PATH variable
     *  - SPAWN_DO_NOT_REAP_CHILD -> We're responsible for catching the child
     *    see child_watch_handler
     */
    static const auto default_flags =
        Glib::SPAWN_SEARCH_PATH | Glib::SPAWN_DO_NOT_REAP_CHILD;

    try {
        // open stdout file first
        if (!_stdout_file.empty()) {
            _file   = Gio::File::create_for_path(_stdout_file);
            _stream = _file->replace("", false, Gio::FILE_CREATE_NONE);
        }

        // spawn it
        PRINT_DEBUG("Spawning process: " << StdUtils::join(_argv));
        Glib::spawn_async_with_pipes(
            "", Glib::ArrayHandle<std::string>(_argv), default_flags,
            sigc::slot<void>(), &_pid, &_stdin, &_stdout, &_stderr);
    } catch (const Glib::Error& ex) {
        EXCEPTION(ex.what());
    }

    // connect childwatch handler
    Glib::signal_child_watch().connect(
        sigc::mem_fun(*this, &ProcessRequest::child_watch_handler), _pid);

    // add io_watches
    _ch_out  = Glib::IOChannel::create_from_fd(_stdout);
    _ch_out->set_encoding ("");
    _ch_out->set_flags (Glib::IO_FLAG_NONBLOCK);
    _ch_err  = Glib::IOChannel::create_from_fd(_stderr);
    _ch_err->set_encoding ("");
    _ch_err->set_flags (Glib::IO_FLAG_NONBLOCK);
    _src_out = _ch_out->create_watch(Glib::IO_IN);
    _src_err = _ch_err->create_watch(Glib::IO_IN);

    _src_out->connect(sigc::mem_fun(*this, &ProcessRequest::handle_stdout));
    _src_err->connect(sigc::mem_fun(*this, &ProcessRequest::handle_stderr));
    _src_out->attach(Glib::MainContext::get_default());
    _src_err->attach(Glib::MainContext::get_default());

    // set timeout, if requested
    if (_timeout > 0)
        Glib::signal_timeout().connect_seconds_once(
            sigc::mem_fun(*this, &ProcessRequest::handle_timeout), _timeout);
}

Glib::RefPtr<ProcessRequest> ProcessRequest::create(
    const std::vector<std::string>& argv, unsigned int timeout,
    bool capture_stdout, const std::string& stdout_file, bool capture_stderr)
{
    return Glib::RefPtr<ProcessRequest>(new ProcessRequest(argv, timeout, capture_stdout, stdout_file, capture_stderr));
}

void
ProcessRequest::set_no_log_error ()
{
    _no_log_error = true;
}

void ProcessRequest::child_watch_handler(GPid pid, int child_status)
{
    Glib::RefPtr<ProcessResult> result;

    PRINT_DEBUG("child_watch_handler()");

    // Read the remaining data from child.
    // The "handle_stdout" might be called after the child gets closed. In that
    // case "_stream " is already closed and output gets lost. Therefore we
    // read all remaining data right now.
    #if 1
    try {
	char buf[1024];
	gsize bytes_read;
        while(_ch_out->read (buf, sizeof (buf), bytes_read) == Glib::IO_STATUS_NORMAL )
        {
	    /* read might return with 0 bytes read,
	     * in that case we just skip
	     */
	    if (bytes_read == 0) {
		continue;
	    }

	    /* read something, encapsulate it into string
	     */
	    std::string buf_string (buf, bytes_read);

            PRINT_DEBUG("Output Line (child): " << Glib::strescape (buf_string));
            PRINT_DEBUG("_capture_stdout: " << _capture_stdout);
            if (_capture_stdout)
                _stdout_buf += buf_string;

            if (!_stdout_file.empty()) {
                gsize bytes_written;
                _stream->write_all(buf_string, bytes_written);
            }
        }
    } catch (const Glib::Error& ex) {
        PRINT_ERROR("Failed to read line from process: " << ex.what());
    }
    try {
	char buf[1024];
	gsize bytes_read;
        while(_ch_err->read (buf, sizeof (buf), bytes_read) == Glib::IO_STATUS_NORMAL )
        {
	    /* read might return with 0 bytes read,
	     * in that case we just skip
	     */
	    if (bytes_read == 0) {
		continue;
	    }

	    /* read something, encapsulate it into string
	     */
	    std::string buf_string (buf, bytes_read);

            if (_capture_stderr)
                _stderr_buf += buf_string;
        }
    } catch (const Glib::Error& ex) {
        PRINT_ERROR("Failed to read from process stderr: " << ex.what());
    }
    #endif

    if (WIFEXITED(child_status))
        // exit normal
        result = ProcessResult::create(
            pid, _stdout_buf, _stderr_buf, true, WEXITSTATUS(child_status), _timeouted, false, -1);
    else if (WIFSIGNALED(child_status))
        // child was signaled
        result = ProcessResult::create(
            pid, _stdout_buf, _stderr_buf, false, -1, _timeouted, true, WTERMSIG(child_status));
    else
        // something else
        result = ProcessResult::create(
            pid, _stdout_buf, _stderr_buf, false, -1, _timeouted, false, -1);

    try {
        std::stringstream ss;
        ss << "Program " << _argv.at(0) << " exited with "
           << (result->success() ? "success" : "failure");
        if (!result->success())
            ss << ": " << result->error_reason();
        auto logger = LogHandler::get_instance();
        logger->log_internal("Info", "process", ss.str());
    } catch (...) {
        PRINT_ERROR("Error while logging...");
    }

    // finish off stdout file
    if (!_stdout_file.empty())
        _stream->close();

    _ch_out->close();
    _ch_err->close();
    _src_out->destroy();
    _src_err->destroy();

    close (_stdin);

    PRINT_DEBUG("Result stdout:     " << result->stdout() );
    PRINT_DEBUG("Result stdout_buf: " << _stdout_buf );
    finished.emit(result);
}

bool ProcessRequest::handle_stdout(const Glib::IOCondition& cond)
{
    UNUSED(cond);

    if (!_capture_stdout && _stdout_file.empty())
        return true;

    try {
	char buf[1024];
	gsize bytes_read;

	/* read to a memory buffer
	 * we want raw data, and no messing with utf-8 here
	 *
	 * the other read methods only returned Glib::ustring
	 */
	Glib::IOStatus stat = _ch_out->read (buf, sizeof (buf), bytes_read);

	if (stat != Glib::IOStatus::IO_STATUS_NORMAL) {
	    PRINT_DEBUG ("ProcessRequest::handle_stdout(): read () returns " << (int) stat);
	    return true;
	}

	if (bytes_read == 0) {
	    PRINT_DEBUG ("ProcessRequest::handle_stdout(): read 0 bytes... exit");
	    return true;
	}

	/* now convert to a string, for easier handling
	 */
	std::string buf_string (buf, bytes_read);

        PRINT_DEBUG("Output Line (stdout): " << Glib::strescape (buf_string));
        if (_capture_stdout)
            _stdout_buf += buf_string;

        if (!_stdout_file.empty()) {
            gsize bytes_written;
            _stream->write_all(buf_string, bytes_written);
        }
    } catch (const Glib::Error& ex) {
        PRINT_ERROR("Failed to read line from process: " << ex.what());
    }

    return true;
}

bool ProcessRequest::handle_stderr(const Glib::IOCondition& cond)
{
    UNUSED(cond);
    Glib::ustring text;

    try {
	char buf[1024];
	gsize bytes_read;

	/* read to a memory buffer
	 * we want raw data, and no messing with utf-8 here
	 *
	 * the other read methods only returned Glib::ustring
	 */
	Glib::IOStatus stat = _ch_err->read (buf, sizeof (buf), bytes_read);

	if (stat != Glib::IOStatus::IO_STATUS_NORMAL) {
	    PRINT_DEBUG ("ProcessRequest::handle_stdout(): read () returns " << (int) stat);
	    return true;
	}

	if (bytes_read == 0) {
	    PRINT_DEBUG ("ProcessRequest::handle_stdout(): read 0 bytes... exit");
	    return true;
	}

	/* now convert to a string, for easier handling
	 */
	std::string buf_string (buf, bytes_read);
	if (_capture_stderr) {
	    _stderr_buf += buf_string;
	} else {
	    /* not capturing stderr,
	     * write it to log
	     */
	    auto logger = LogHandler::get_instance();
	    if (_no_log_error) {
		logger->log_internal("Debug", "process", buf_string);
	    } else {
		logger->log_internal("Error", "process", buf_string);
	    }
	}
    } catch (...) {
        PRINT_ERROR("Error while logging...");
    }

    return true;
}

void ProcessRequest::handle_timeout()
{
    kill(_pid, SIGKILL);
    _timeouted = true;
}
