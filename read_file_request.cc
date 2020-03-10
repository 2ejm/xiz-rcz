#include "read_file_request.h"
#include "utils.h"

Glib::RefPtr<ReadFileRequest> ReadFileRequest::create(const std::string& path)
{
    return Glib::RefPtr<ReadFileRequest>(new ReadFileRequest(path));
}

void ReadFileRequest::start_read()
{
    PRINT_DEBUG ("ReadFileRequest::start_read()");
    _file = Gio::File::create_for_path(_path);
    _file->load_contents_async(
        sigc::mem_fun(*this, &ReadFileRequest::on_async_ready));
}

void ReadFileRequest::on_async_ready(const Glib::RefPtr<Gio::AsyncResult>& result)
{
    std::string content;
    PRINT_DEBUG ("ReadFileRequest::on_async_ready()");

    try {
        char *buffer;
        gsize length;

        _file->load_contents_finish(result, buffer, length);
        content.insert(0, buffer, length);

        g_free(buffer);
    } catch (const Glib::Error& ex) {
        auto signal = ReadFileResult::create("", true, ex.what());
        finished.emit(signal);
        return;
    }

    auto signal = ReadFileResult::create(content);
    finished.emit(signal);
}
