#include "write_file_request.h"

Glib::RefPtr<WriteFileRequest>
WriteFileRequest::create(const std::string& path, const std::string& content)
{
    return Glib::RefPtr<WriteFileRequest>(new WriteFileRequest(path, content));
}

void WriteFileRequest::start_write()
{
    std::string etag;

    _file = Gio::File::create_for_path(_path);
    _file->replace_contents_async(
        sigc::mem_fun(*this, &WriteFileRequest::on_async_ready), _content, etag,
        false, Gio::FILE_CREATE_NONE);
}

void WriteFileRequest::on_async_ready(const Glib::RefPtr<Gio::AsyncResult>& result)
{
    try {
        _file->replace_contents_finish(result);
    } catch (const Glib::Error& ex) {
        auto signal = WriteFileResult::create(true, ex.what());
        finished.emit(signal);
        return;
    }

    auto signal = WriteFileResult::create();
    finished.emit(signal);
}
