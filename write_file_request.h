#ifndef _WRITE_FILE_REQUEST_H_
#define _WRITE_FILE_REQUEST_H_

#include <string>

#include <glibmm/object.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <giomm/file.h>

#include <sigc++/signal.h>

#include "write_file_result.h"

/**
 * This class be used to write a complete file async. The end of the operation
 * will be channeled by the `finish` signal.
 */
class WriteFileRequest : public Glib::Object
{
public:
    WriteFileRequest(const std::string& path, const std::string& content) :
        _path(path), _content(content)
    {}

    static Glib::RefPtr<WriteFileRequest>
    create(const std::string& path, const std::string& content);

    void start_write();

    sigc::signal<void, const Glib::RefPtr<WriteFileResult>& > finished;

private:
    Glib::RefPtr<Gio::File> _file;
    std::string _path;
    std::string _content;

    void on_async_ready(const Glib::RefPtr<Gio::AsyncResult>& result);
};

#endif /* _WRITE_FILE_REQUEST_H_ */
