#include "xml_helpers.h"
#include "file_handler.h"
#include "log_handler.h"
#include "ustring_utils.h"
#include "utils.h"

#include "update_signature_check_request.h"

const std::vector<std::string> UpdateSignatureCheckRequest::default_openssl_args =
{ "openssl", "dgst", "-sha256", "-verify", "/etc/swupdate/public.pem", "-signature" };

UpdateSignatureCheckRequest::UpdateSignatureCheckRequest (const Glib::ustring & content_fname, const Glib::ustring & signature)
    : _content_fname (content_fname)
    , _signature (signature)
{ }

Glib::RefPtr <UpdateSignatureCheckRequest>
UpdateSignatureCheckRequest::create (const Glib::ustring & content_fname, const Glib::ustring & signature)
{
    return Glib::RefPtr <UpdateSignatureCheckRequest> (new UpdateSignatureCheckRequest (content_fname, signature));
}

void UpdateSignatureCheckRequest::cleanup() const
{
    // cleanup temporary files
    try {
        if (!_sig_file.empty())
            FileHandler::del_file(_sig_file);
    } catch (...) {
        PRINT_ERROR("Couldn't remove temporary files...");
    }
}

void UpdateSignatureCheckRequest::verify_via_openssl()
{
    std::vector<std::string> args(default_openssl_args);

    args.emplace_back(_sig_file);
    args.emplace_back(_content_fname);

    _proc = ProcessRequest::create(args, ProcessRequest::DEFAULT_TIMEOUT);
    _proc->finished.connect(
        sigc::mem_fun(*this, &UpdateSignatureCheckRequest::on_proc_finish));
    _proc->start_process();
}

void UpdateSignatureCheckRequest::on_proc_finish(const Glib::RefPtr<ProcessResult>& result)
{
    cleanup();

    if (!result->success()) {
        finished.emit(
            SignatureCheckResult::create(
                false, Glib::ustring::compose("openssl verification failed: %1",
                                              result->error_reason())));
        return;
    }

    finished.emit(SignatureCheckResult::create(true, ""));
}

void UpdateSignatureCheckRequest::start_check()
{
    Glib::RefPtr<Gio::UnixOutputStream> temp_stream;
    std::string error_msg;
    bool error = false;

    try {
        Glib::ustring decoded_signature;
	gsize bytes_written;

	/* write decoded signature to temp_stream
	 */
        temp_stream = FileHandler::get_temp_file_write(_sig_file);
	decoded_signature = FileHandler::base64_decode (_signature);
        temp_stream->write_all(decoded_signature.data(), decoded_signature.bytes(), bytes_written);
        temp_stream->close();

        /* now call openssl to verify the signature
	 */
        verify_via_openssl();
    } catch (const std::exception& ex) {
        error     = true;
        error_msg = ex.what();
    } catch (const Glib::Error& ex) {
        error     = true;
        error_msg = ex.what();
    }

    /* upon failure, the finished signal must get
     * emitted immediately.
     *
     * Failure to verify signature is always a false Result
     */
    if (error) {
        cleanup();
        finished.emit(SignatureCheckResult::create(false, error_msg));
    }

    /* the verification process is running now,
     * checking will continue in UpdateSignatureCheckRequest::on_proc_finish()
     */
    return;
}
