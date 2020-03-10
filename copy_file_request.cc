#include "copy_file_request.h"

Glib::RefPtr<CopyFileRequest>
CopyFileRequest::create(const std::string& src, const std::string& dest)
{
    return Glib::RefPtr<CopyFileRequest>(new CopyFileRequest(src, dest));
}

void CopyFileRequest::start_copy()
{
    _file_src = Gio::File::create_for_path(_src);
    _file_dest = Gio::File::create_for_path(_dest);
    _file_src->copy_async(_file_dest,  sigc::slot<void, goffset, goffset>(),
                           sigc::mem_fun(*this, &CopyFileRequest::on_async_ready),
                           Gio::FILE_COPY_OVERWRITE, Glib::PRIORITY_DEFAULT);
}

void CopyFileRequest::on_async_ready(const Glib::RefPtr<Gio::AsyncResult>& result)
{
    try {
        _file_src->copy_finish(result);
    } catch (const Glib::Error& ex) {
        auto signal = CopyFileResult::create(true, ex.what());
        finished.emit(signal);
        return;
    }

    auto signal = CopyFileResult::create();
    finished.emit(signal);
}
