#ifndef _PROCESS_RESULT_H_
#define _PROCESS_RESULT_H_

#include <glibmm/object.h>
#include <glibmm/spawn.h>
#include <glibmm/main.h>
#include <glibmm/refptr.h>

#include <sstream>
#include <string>

/**
 * This class represents an ProcessResult.  If you execute an external process
 * like `perl`, you can use a ProcessRequest for that. If you subscribe to the
 * `finished` signal, you'll be informed about the pid and exit status of your
 * process request.
 *
 * This class contains the necessary values for that.
 */
class ProcessResult : public Glib::Object
{
public:
    ProcessResult(
        GPid pid, const std::string & stdout, const std::string & stderr,
	bool exited_normally, int child_status,
        bool timeouted = false, bool signaled = false, int signal = -1) :
        _pid{pid}, _stdout{stdout}, _stderr{stderr}, _exited_normally{exited_normally},
        _child_status{child_status}, _timeouted{timeouted}, _signaled{signaled},
        _signal{signal}
    {}

    static Glib::RefPtr<ProcessResult> create(
        GPid pid, const std::string & stdout, const std::string & stderr,
	bool exited_normally, int child_status,
        bool timeouted = false, bool signaled = false, int signal = 0)
    {
        return Glib::RefPtr<ProcessResult>(
            new ProcessResult(
                pid, stdout, stderr, exited_normally, child_status, timeouted, signaled, signal));
    }

    GPid get_pid() const noexcept
    {
        return _pid;
    }

    const std::string& stdout() const noexcept
    {
        return _stdout;
    }

    std::string& stdout() noexcept
    {
        return _stdout;
    }

    const std::string& stderr() const noexcept
    {
        return _stderr;
    }

    std::string& stderr() noexcept
    {
        return _stderr;
    }

    bool get_exited_normally() const noexcept
    {
        return _exited_normally;
    }

    int get_child_status() const noexcept
    {
        return _child_status;
    }

    bool get_timeouted() const noexcept
    {
        return _timeouted;
    }

    bool get_signaled() const noexcept
    {
        return _signaled;
    }

    int get_signal() const noexcept
    {
        return _signal;
    }

    bool success() const noexcept
    {
        return _exited_normally && _child_status == 0;
    }

    std::string error_reason() const
    {
        std::stringstream ss;

        if (_exited_normally)
            ss << "exited with status: " << _child_status;
        else if (_signaled)
            ss << "exited with signal: " << _signal;
        else
            ss << "exited due to unknown reason";

        return ss.str();
    }

private:
    GPid _pid;
    std::string _stdout;
    std::string _stderr;
    bool _exited_normally;
    int  _child_status;
    bool _timeouted;           // Killed b/o timeout?
    bool _signaled;
    int  _signal;
};

#endif /* _PROCESS_RESULT_H_ */
