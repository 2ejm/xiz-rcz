#include "xml_helpers.h"
#include "file_handler.h"
#include "log_handler.h"
#include "ustring_utils.h"
#include "utils.h"

#include "signature_check_request.h"

const std::vector<std::string> SignatureCheckRequest::default_gpg_args =
{ "gpg", "--verify", "--keyring", "/etc/zix/keyring.gpg", "--ignore-time-conflict", "--no-default-keyring" };

// Note: This contains only the true values, everything else is considered false
const SignatureCheckRequest::SigCheckMap SignatureCheckRequest::sig_check_map =
{
    { STR_ZIXINF_IPC, {
            { "guiIO", true },
        },
    },
    { STR_ZIXINF_LANWEBSERVICE, {
	    { "update", true },
	    { "getTemplate", true },
	    { "getFilesList", true },
	    { "getFile", true },
	    { "setCalibration", true },
	    { "setDefaults", true },
	    { "setTemplatesList", true },
	    { "setTemplate", true },
	    { "setFile", true },
	    { "delTemplatesList", true },
	    { "delFile", true },
	    { "guiIO", true },
	},
    },
    { STR_ZIXINF_LANSHAREDFOLDER, {
            { "update", true },
            { "procedure", true },
            { "getTemplate", true },
            { "getFilesList", true },
            { "getFile", true },
            { "setCalibration", true },
            { "setDefaults", true },
            { "setTemplatesList", true },
            { "setTemplate", true },
            { "setFile", true },
            { "delTemplatesList", true },
            { "delFile", true },
            { "guiIO", true },
        },
    },
    { STR_ZIXINF_LANSOCKET, {
            { "update", true },
            { "getTemplate", true },
            { "getFilesList", true },
            { "getFile", true },
            { "setCalibration", true },
            { "setDefaults", true },
            { "setTemplatesList", true },
            { "setTemplate", true },
            { "setFile", true },
            { "delTemplatesList", true },
            { "delFile", true },
            { "guiIO", true },
        },
    },
    { STR_ZIXINF_USB, {
            { "update", true },
            { "procedure", true },
            { "getTemplate", true },
            { "getFilesList", true },
            { "getFile", true },
            { "setCalibration", true },
            { "setDefaults", true },
            { "setTemplatesList", true },
            { "setTemplate", true },
            { "setFile", true },
            { "delTemplatesList", true },
            { "delFile", true },
            { "guiIO", true },
        },
    },
    { STR_ZIXINF_COM1, {
            { "update", true },
       },
    },
    { STR_ZIXINF_COM2, {
            { "update", true },
       },
    },
};

Glib::ustring SignatureCheckRequest::get_signature() const
{
    return _signature;
}

Glib::ustring SignatureCheckRequest::get_xml_without_signature() const
{
    xmlpp::Document doc;

    doc.create_root_node_by_import(_root, true);
    auto *root = doc.get_root_node();
    auto *sig_child = root->get_first_child("signature");
    root->remove_child(sig_child);

    return doc.write_to_string();
}

bool SignatureCheckRequest::check_needed(const ZixInterface& channel) const
{
    auto stage1 = sig_check_map.find(channel.to_string());

    if (stage1 == sig_check_map.end()) {
        PRINT_DEBUG("Unknown Interface used: " << channel.to_string());
        return false;
    }

    auto stage2 = stage1->second.find(_fid);
    if (stage2 == stage1->second.end())
        return false;

    return stage2->second;
}

void SignatureCheckRequest::cleanup() const
{
    // cleanup temporary files
    try {
        if (!_sig_file.empty())
            FileHandler::del_file(_sig_file);
        if (!_xml_file.empty())
            FileHandler::del_file(_xml_file);
    } catch (...) {
        PRINT_ERROR("Couldn't remove temporary files...");
    }
}

void SignatureCheckRequest::verify_via_gpg()
{
    std::vector<std::string> gpg_args(default_gpg_args);

    gpg_args.emplace_back(_sig_file);
    gpg_args.emplace_back(_xml_file);

    _gpg_proc = ProcessRequest::create(gpg_args, ProcessRequest::DEFAULT_TIMEOUT);
    _gpg_proc->finished.connect(
        sigc::mem_fun(*this, &SignatureCheckRequest::on_gpg_proc_finish));
    _gpg_proc->start_process();
}

void SignatureCheckRequest::on_gpg_proc_finish(const Glib::RefPtr<ProcessResult>& result)
{
    cleanup();

    if (!result->success()) {
        finished.emit(
            SignatureCheckResult::create(
                false, Glib::ustring::compose("Gpg verification failed: %1",
                                              result->error_reason())));
        return;
    }

    finished.emit(SignatureCheckResult::create(true, ""));
}

void SignatureCheckRequest::start_check(const ZixInterface& channel)
{
    Glib::RefPtr<Gio::UnixOutputStream> temp_stream1, temp_stream2;
    std::string error_msg;
    bool error = false;

    if (!check_needed(channel)) {
        finished.emit(SignatureCheckResult::create(true, ""));
        return;
    }

    try {
        gsize bytes_written;
        Glib::ustring signature, xml_without_sig;

        // 1. Open two temporary files
        temp_stream1 = FileHandler::get_temp_file_write(_sig_file);
        temp_stream2 = FileHandler::get_temp_file_write(_xml_file);

        // 2. Get signature and write to first file
        signature = get_signature();
        if (signature.empty())
            EXCEPTION("Signature tag not found in XML request");
        if (use_binary_sig)
            signature = FileHandler::base64_decode(signature);
        temp_stream1->write_all(signature.data(), signature.bytes(), bytes_written);

        // 3. Get Fid XML without signature tag and write to second file
        xml_without_sig = get_xml_without_signature();
        temp_stream2->write_all(xml_without_sig.data(), xml_without_sig.bytes(), bytes_written);

        temp_stream1->close();
        temp_stream2->close();

        // 4. Call gpg in order to verify XML by signature
        verify_via_gpg();
    } catch (const std::exception& ex) {
        error     = true;
        error_msg = ex.what();
    } catch (const Glib::Error& ex) {
        error     = true;
        error_msg = ex.what();
    }

    // failure?
    if (error) {
        cleanup();
        finished.emit(SignatureCheckResult::create(false, error_msg));
    }

    // success!
    return;
}
