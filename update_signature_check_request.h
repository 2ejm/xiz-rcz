#ifndef _UPDATE_SIGNATURE_CHECK_REQUEST_H_
#define _UPDATE_SIGNATURE_CHECK_REQUEST_H_

#include <vector>
#include <string>
#include <map>

#include <glibmm/ustring.h>
#include <glibmm/object.h>
#include <glibmm/refptr.h>
#include <glibmm/fileutils.h>
#include <giomm/unixoutputstream.h>

#include <libxml++/nodes/element.h>
#include <libxml++/parsers/domparser.h>

#include <sigc++/signal.h>

#include "signature_check_result.h"
#include "process_request.h"
#include "process_result.h"
#include "xml_function.h"
#include "xml_image.h"
#include "zix_interface.h"

/**
 * This class can be used in order to check whether the signature for
 * a given <image /> is valid.
 *
 * We use openssl to achieve that:
 *
 * -> openssl dgst -sha256 -verify /etc/swupdate/public.pem -signature <signature> <file>
 */
class UpdateSignatureCheckRequest : public Glib::Object
{
public:
    UpdateSignatureCheckRequest (const Glib::ustring & content_fname, const Glib::ustring & signature);

    static Glib::RefPtr <UpdateSignatureCheckRequest> create (const Glib::ustring & content_fname, const Glib::ustring & signature);

    void start_check();

    sigc::signal<void, const Glib::RefPtr<SignatureCheckResult>& > finished;

private:
    static const std::vector<std::string> default_openssl_args;

    Glib::RefPtr <ProcessRequest> _proc;

    Glib::ustring _content_fname;
    Glib::ustring _signature;

    std::string _sig_file;

    Glib::ustring get_signature() const;
    void verify_via_openssl();
    void cleanup() const;

    void on_proc_finish(const Glib::RefPtr<ProcessResult>& result);
};

#endif /* _UPDATE_SIGNATURE_CHECK_REQUEST_H_ */
