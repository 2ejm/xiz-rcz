#ifndef _READ_FILE_REQUEST_H_
#define _READ_FILE_REQUEST_H_

#include <string>

#include <glibmm/object.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <glibmm/arrayhandle.h>
#include <glibmm/spawn.h>
#include <glibmm/main.h>
#include <glibmm/iochannel.h>
#include <giomm/file.h>

#include <sigc++/signal.h>

#include "read_file_result.h"

/**
 * This class be used to read a complete file async. The end of the operation
 * will be channeled by the `finish` signal.
 */
class ReadFileRequest : public Glib::Object
{
public:
    explicit ReadFileRequest(const std::string& path) :
        _path(path)
    {}

    static Glib::RefPtr<ReadFileRequest> create(const std::string& path);

    void start_read();

    sigc::signal<void, const Glib::RefPtr<ReadFileResult>& > finished;

private:
    Glib::RefPtr<Gio::File> _file;
    std::string _path;

    void on_async_ready(const Glib::RefPtr<Gio::AsyncResult>& result);
};

#endif /* _READ_FILE_REQUEST_H_ */
