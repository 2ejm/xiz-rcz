#include <fstream>
#include <istream>
#include <sstream>

#include "file_interface_connection.h"
#include "file_handler.h"

#include "utils.h"

FileInterfaceConnection::FileInterfaceConnection(const std::string& in_path,
                                                 const std::string& content,
                                                 const std::string& out_path) :
    InterfaceConnection(out_path),
    _in_path{in_path},
    _content{content},
    _out_path{out_path}
{
    PRINT_DEBUG ("FileInterfaceConnection::FileInterfaceConnection ("
                 << in_path << ", " << content << ", " << out_path << ")");

    // save input filename for function calls which need access to it
    _filename = _in_path;
}

void FileInterfaceConnection::read_file()
{
    if (_in_path.empty())
        EXCEPTION("No input path given.");

    std::ifstream is(_in_path);

    if (!is.good())
        EXCEPTION("Failed to open file: " << _in_path);

    request_ready.emit(is, 0);
}

void FileInterfaceConnection::stream_complete()
{
    if (_content.empty())
        EXCEPTION("No content available.");

    std::stringbuf buf;
    buf.str(_content);

    std::istream is(&buf);

    request_ready.emit(is, 0);
}

void FileInterfaceConnection::emit_result(const Glib::ustring& result, int tid)
{
    (void) tid;

    PRINT_DEBUG ("FileInterfaceConnection::emit_result() out_path=" << _out_path);
    if (_out_path.empty())
        goto out;

    try {
	FileHandler::set_file (_out_path, result);
    } catch (std::exception & e) {
	PRINT_ERROR ("FileInterfaceConnection::emit_result() failed to write to path");

	PRINT_DEBUG ("logging result to log");
	PRINT_DEBUG (result);

	/* we just couldnt drop our result, not so bad
	 * just exit now
	 */
    }

out:
    connection_closed.emit();
}

void FileInterfaceConnection::cancel()
{}
