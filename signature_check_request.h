#ifndef _SIGNATURE_CHECK_REQUEST_H_
#define _SIGNATURE_CHECK_REQUEST_H_

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
#include "zix_interface.h"

/**
 * This class can be used in order to check whether the signature for
 * a given <function /> is valid.
 *
 * We use gpg to achieve that:
 *
 * -> gpg --verify --keyring /etc/zix/keyring.gpg --no-default-keyring <signature_file> <xml_file>
 */
class SignatureCheckRequest : public Glib::Object
{
public:
    using RefPtr      = Glib::RefPtr<SignatureCheckRequest>;
    using SigCheckMap = std::map<std::string, std::map<std::string, bool> >;

    inline SignatureCheckRequest (const Glib::ustring & signature,
				  const ZixInterface & channel,
				  const Glib::ustring & fid,
				  const xmlpp::Element *root)
	: _signature{signature}
	, _channel{channel}
	, _fid{fid}
	, _root{root}
    {}

    static inline RefPtr create (const Glib::ustring & signature,
				 const ZixInterface & channel,
				 const Glib::ustring & fid,
				 const xmlpp::Element *root)
    {
        return RefPtr(new SignatureCheckRequest(signature, channel, fid, root));
    }

    void start_check(const ZixInterface& channel);

    sigc::signal<void, const Glib::RefPtr<SignatureCheckResult>& > finished;

private:
    static const std::vector<std::string> default_gpg_args;
    static const SigCheckMap sig_check_map;
    /**
     * Gpg signatures can be ascii (.asc) or binary (.sig). We expect binary
     * ones in Base64 encoded format.
     *
     * true  -> use binary signatures
     * false -> use ascii  signatures
     */
    static constexpr bool use_binary_sig = true;

    Glib::RefPtr<ProcessRequest> _gpg_proc;
    Glib::ustring _signature;
    ZixInterface _channel;
    Glib::ustring _fid;
    const xmlpp::Element *_root;
    std::string _sig_file;
    std::string _xml_file;

    Glib::ustring get_signature() const;
    Glib::ustring get_xml_without_signature() const;
    void write_xml_without_sig_tag() const;
    void verify_via_gpg();
    bool check_needed(const ZixInterface& channel) const;
    void cleanup() const;

    void on_gpg_proc_finish(const Glib::RefPtr<ProcessResult>& result);
};

#endif /* _SIGNATURE_CHECK_REQUEST_H_ */
