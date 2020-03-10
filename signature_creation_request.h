#ifndef _SIGNATURE_CREATION_REQUEST_H_
#define _SIGNATURE_CREATION_REQUEST_H_

#include <string>
#include <vector>

#include <glibmm/ustring.h>
#include <glibmm/object.h>
#include <glibmm/refptr.h>
#include <glibmm/fileutils.h>
#include <giomm/unixoutputstream.h>

#include <libxml++/nodes/element.h>
#include <libxml++/parsers/domparser.h>

#include <sigc++/signal.h>

#include "signature_creation_result.h"
#include "process_request.h"
#include "process_result.h"

/**
 * This class can be used in order to create a signature for a given XML file.
 * The XML file should have a <signature>asd</signature> with a bogus value. The
 * file will be overridden and contain the valid signature after
 * SignatureCreationRequest will be successful.
 *
 * We use gpg to achieve that:
 *
 * -> gpg --keyring /etc/zix/keyring.gpg --no-default-keyring --output <signature_file> --detach-sig <xml_file>
 */
class SignatureCreationRequest : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<SignatureCreationRequest>;
    enum Mode { MODE_GPG, MODE_HMAC };

    inline SignatureCreationRequest(const std::string& xml_file, Mode mode)
	: _xml_file{xml_file}
	, _mode{mode}
    {}

    static inline RefPtr create(const std::string& xml_file, Mode mode)
    {
        return RefPtr(new SignatureCreationRequest(xml_file, mode));
    }

    void start_creation();

    sigc::signal<void, const Glib::RefPtr<SignatureCreationResult>& > finished;

private:
    static const std::vector<std::string> default_gpg_args;
    static const std::vector<std::string> default_hmac_args;

    Glib::RefPtr<ProcessRequest> _sig_proc;
    std::string _xml_file;
    std::string _sig_file;
    std::string _xml_without_sig_file;
    Mode _mode;

    Glib::ustring get_signature() const;
    Glib::ustring get_hmac_signature() const;
    void write_xml_without_signature();
    void create_signature_via_gpg();
    void create_signature_via_hmac();
    void exchange_signature_in_xml(const Glib::ustring & signature) const;
    void cleanup() const;

    void on_gpg_proc_finish(const Glib::RefPtr<ProcessResult>& result);
    void on_hmac_proc_finish(const Glib::RefPtr<ProcessResult>& result);
};

#endif /* _SIGNATURE_CREATION_REQUEST_H_ */
