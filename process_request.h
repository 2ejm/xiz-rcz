#ifndef _PROCESS_REQUEST_H_
#define _PROCESS_REQUEST_H_

#include <string>

#include <glibmm/object.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <glibmm/arrayhandle.h>
#include <glibmm/spawn.h>
#include <glibmm/main.h>
#include <glibmm/iochannel.h>

#include <giomm/file.h>
#include <giomm/fileoutputstream.h>

#include <sigc++/signal.h>

#include "process_result.h"

/**
 * This class wraps a process request. It uses Glib::spawn (which uses
 * fork/exec) in order to create a new process, handle exit, I/O and
 * timeouts. The process is launched async., but you can connect() to the
 * `finish` signal to receive the pid and exit status. Stdout/Stderr are
 * redirected to the LogHandler instance.
 *
 * Example of usage:
 *   auto req = ProcessRequest::create(std::vector<std::string>{"ping", "8.8.8.8"});
 *   req->finish.connect(sigc::ptr_fun(&foobar));
 *   req->start_process();
 */
class ProcessRequest : public Glib::Object
{
public:
    static const unsigned DEFAULT_TIMEOUT = ( 60 * 10 );

    ProcessRequest(const std::vector<std::string>& argv, unsigned timeout = 0,
                   bool capture_stdout = false, const std::string& stdout_file = "",
		   bool capture_stderr = false);

    /**
     * Creates an process request. If the timeout is non-zero, then after
     * timeout the process will be killed by SIG_KILL and the timeouted flag is
     * set in the process result.
     *
     * @param argv           e.g. { "echo", "hallo" }
     * @param timeout        the timeout intervall in seconds
     * @param capture_stdout if true -> stdout will be captured and can be accessed by result
     * @param stdout_file    if set  -> stdout will be written into that file (think of a pipe)
     *
     * @return process request instance
     */
    static Glib::RefPtr<ProcessRequest> create(
        const std::vector<std::string>& argv, unsigned int timeout = 0,
        bool capture_stdout = false, const std::string& stdout_file = "",
	bool capture_stderr = false);

    /**
     * Starts the process async. You can connect the `finished` signal to be
     * informed about the child exit status.
     */
    void start_process();

    /**
     * set no_log_error flag.
     * When this flag is set, no error is logged, when
     * the process returns with an error
     */
    void set_no_log_error ();

    sigc::signal<void, const Glib::RefPtr<ProcessResult>& > finished;

private:
    Glib::RefPtr<Gio::File> _file;
    Glib::RefPtr<Gio::FileOutputStream> _stream;
    std::vector<std::string> _argv;
    std::string _stdout_buf;
    std::string _stdout_file;
    std::string _stderr_buf;
    Glib::RefPtr<Glib::IOChannel> _ch_out;
    Glib::RefPtr<Glib::IOChannel> _ch_err;
    Glib::RefPtr<Glib::IOSource> _src_out;
    Glib::RefPtr<Glib::IOSource> _src_err;
    GPid _pid;
    int _stdin;
    int _stdout;
    int _stderr;
    bool _timeouted;
    bool _capture_stdout;
    bool _capture_stderr;
    unsigned _timeout;
    bool _no_log_error;

    void child_watch_handler(GPid pid, int child_status);
    bool handle_stdout(const Glib::IOCondition& cond);
    bool handle_stderr(const Glib::IOCondition& cond);
    void handle_timeout();
};

#endif /* _PROCESS_REQUEST_H_ */
