#ifndef _COPY_FILE_REQUEST_H_
#define _COPY_FILE_REQUEST_H_

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

#include "copy_file_result.h"

/**
 * This class be used to copy a complete file async. The end of the operation
 * will be channeled by the `finish` signal.
 */
class CopyFileRequest : public Glib::Object
{
public:
    explicit CopyFileRequest(const std::string& src, const std::string& dest) :
        _src(src), _dest(dest)
    {}

    static Glib::RefPtr<CopyFileRequest>
    create(const std::string& src, const std::string& dest);

    void start_copy();

    sigc::signal<void, const Glib::RefPtr<CopyFileResult>& > finished;

private:
    Glib::RefPtr<Gio::File> _file_src;
    Glib::RefPtr<Gio::File> _file_dest;
    std::string _src;
    std::string _dest;

    void on_async_ready(const Glib::RefPtr<Gio::AsyncResult>& result);
};

#endif /* _COPY_FILE_REQUEST_H_ */
