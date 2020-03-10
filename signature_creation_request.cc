#include "xml_helpers.h"
#include "file_handler.h"
#include "log_handler.h"
#include "ustring_utils.h"
#include "utils.h"
#include "conf_handler.h"

#include "signature_creation_request.h"

// FIXME: Define keyring settings for key creation
const std::vector<std::string> SignatureCreationRequest::default_gpg_args =
{ "gpg", "--batch", "--yes", "--keyring", "/etc/zix/keyring_signing.gpg", "--no-default-keyring" };

const std::vector<std::string> SignatureCreationRequest::default_hmac_args =
{ "openssl", "dgst", "-sha1" };

Glib::ustring SignatureCreationRequest::get_signature() const
{
    auto content = FileHandler::get_file(_sig_file);
    return FileHandler::base64_encode(content);
}

Glib::ustring SignatureCreationRequest::get_hmac_signature() const
{
    auto content = FileHandler::get_file(_sig_file);
    auto s = content.rfind(' ');
    return FileHandler::base64_encode(content.substr(s+1));
}

void SignatureCreationRequest::write_xml_without_signature()
{
    xmlpp::DomParser parser;

    // get xml and remove signature tag
    parser.parse_file(_xml_file);
    auto *doc = parser.get_document();
    if (!doc)
        EXCEPTION("Invalid input XML");
    auto *root = doc->get_root_node();
    if (!root)
        EXCEPTION("Invalid input XML");
    auto *sig_child = root->get_first_child("signature");
    if (!sig_child)
        EXCEPTION("Invalid input XML");
    root->remove_child(sig_child);

    // get temp file
    auto stream = FileHandler::get_temp_file_write(_xml_without_sig_file);
    stream->close();

    // write xml without signature tag
    doc->write_to_file(_xml_without_sig_file);

    return;
}

void SignatureCreationRequest::cleanup() const
{
    // cleanup temporary files
    try {
        if (!_sig_file.empty() &&
            FileHandler::file_exists(_sig_file))
            FileHandler::del_file(_sig_file);
        if (!_xml_without_sig_file.empty() &&
            FileHandler::file_exists(_xml_without_sig_file))
            FileHandler::del_file(_xml_without_sig_file);
    } catch (...) {
        PRINT_ERROR("Couldn't remove temporary files...");
    }
}

void SignatureCreationRequest::create_signature_via_hmac()
{
    std::vector<std::string> hmac_args(default_hmac_args);
    auto conf = ConfHandler::get_instance();
    auto mac_addr=conf->getConfParameter("macAddress");

    // get temp file for signature
    auto stream = FileHandler::get_temp_file_write(_sig_file);
    stream->close();

    // call gpg
    hmac_args.emplace_back("-hmac");
    hmac_args.emplace_back(mac_addr);
    hmac_args.emplace_back(_xml_without_sig_file);

    _sig_proc = ProcessRequest::create(hmac_args, ProcessRequest::DEFAULT_TIMEOUT, true, _sig_file);
    _sig_proc->finished.connect(
        sigc::mem_fun(*this, &SignatureCreationRequest::on_hmac_proc_finish));
    _sig_proc->start_process();
}

void SignatureCreationRequest::on_hmac_proc_finish(const Glib::RefPtr<ProcessResult>& result)
{
    if (!result->success()) {
        cleanup();
        finished.emit(
            SignatureCreationResult::create(
                false, Glib::ustring::compose(
                    "Signature creation via openssl failed: %1", result->error_reason())));
        return;
    }

    try {
        exchange_signature_in_xml (get_hmac_signature ());
    } catch (const std::exception& ex) {
        cleanup();
        finished.emit(SignatureCreationResult::create(false, ex.what()));
        return;
    }

    cleanup();
    finished.emit(SignatureCreationResult::create(true, ""));
}

void SignatureCreationRequest::create_signature_via_gpg()
{
    std::vector<std::string> gpg_args(default_gpg_args);

    // get temp file for signature
    auto stream = FileHandler::get_temp_file_write(_sig_file);
    stream->close();

    // call gpg
    gpg_args.emplace_back("--output");
    gpg_args.emplace_back(_sig_file);
    gpg_args.emplace_back("--detach-sig");
    gpg_args.emplace_back(_xml_without_sig_file);

    _sig_proc = ProcessRequest::create(gpg_args, ProcessRequest::DEFAULT_TIMEOUT);
    _sig_proc->finished.connect(
        sigc::mem_fun(*this, &SignatureCreationRequest::on_gpg_proc_finish));
    _sig_proc->start_process();
}



void SignatureCreationRequest::on_gpg_proc_finish(const Glib::RefPtr<ProcessResult>& result)
{
    if (!result->success()) {
        cleanup();
        finished.emit(
            SignatureCreationResult::create(
                false, Glib::ustring::compose(
                    "Signature creation via gpg failed: %1", result->error_reason())));
        return;
    }

    try {
        exchange_signature_in_xml (get_signature ());
    } catch (const std::exception& ex) {
        cleanup();
        finished.emit(SignatureCreationResult::create(false, ex.what()));
        return;
    }

    cleanup();
    finished.emit(SignatureCreationResult::create(true, ""));
}

void SignatureCreationRequest::exchange_signature_in_xml(const Glib::ustring & signature) const
{
    xmlpp::DomParser parser;

    // replace signature in original xml
    parser.parse_file(_xml_file);
    auto *doc = parser.get_document();
    if (!doc)
        EXCEPTION("Invalid input XML");
    auto *root = doc->get_root_node();
    if (!root)
        EXCEPTION("Invalid input XML");
    auto *sig_child = root->get_first_child("signature");
    if (!sig_child)
        EXCEPTION("Invalid input XML");
    auto *en = dynamic_cast<xmlpp::Element *>(sig_child);
    if (!en)
        EXCEPTION("Invalid input XML");
    en->set_child_text(signature);

    // write back
    doc->write_to_file(_xml_file);
}

void SignatureCreationRequest::start_creation()
{
    try {
        write_xml_without_signature();
	switch (_mode) {
	    case MODE_GPG:
		create_signature_via_gpg();
		break;
	    case MODE_HMAC:
		create_signature_via_hmac();
		break;
	}
    } catch (const std::exception& ex) {
        finished.emit(SignatureCreationResult::create(false, ex.what()));
    }
}
