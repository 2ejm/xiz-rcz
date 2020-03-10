#ifndef ZIX_XML_REQUEST_H
#define ZIX_XML_REQUEST_H

#include <glibmm/object.h>
#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include <libxml++/parsers/domparser.h>

#include <memory>

#include "xml_result.h"
#include "xml_function.h"
#include "restriction_check_result.h"
#include "restriction_check_request.h"
#include "signature_check_result.h"
#include "signature_check_request.h"
#include "interface_connection.h"
#include "zix_interface.h"
#include "function_call.h"

/**
 * \brief represents xml received from InterfaceHandler
 *
 * Create XmlRequest from std::istream.
 * Also stores the Reference to XmlFunction representation.
 * And also the CoreFunctionCall
 *
 * handled/managed by XmlProcessor
 */
class XmlRequest : public Glib::Object
{
public:
    XmlRequest (std::istream & is, int prio, std::weak_ptr <InterfaceConnection> ic, const Glib::ustring& interface, int tid, bool restart_proc);
    static Glib::RefPtr <XmlRequest> create (std::istream & is, int prio, std::weak_ptr <InterfaceConnection> ih, const Glib::ustring& interface, int tid, bool restart_proc = false);

    sigc::signal <void> finished;
    void execute ();

    int prio() const noexcept;
    int ts() const noexcept;

    Glib::ustring fid() const noexcept;

protected:
    xmlpp::DomParser _parser;

    Glib::RefPtr <XmlFunction> _function;
    Glib::RefPtr <FunctionCall> _current_call;
    Glib::RefPtr <RestrictionCheckRequest> _rest_check;
    Glib::RefPtr <SignatureCheckRequest> _sig_check;

    void print_node (const xmlpp::Node *node);
    void print_dom ();

    void function_finished (Glib::RefPtr <XmlResult> result);

private:
    static int req_counter;
    int _prio;
    std::weak_ptr <InterfaceConnection> _interface_connection;
    Glib::ustring _interface;
    int _tid;
    int _ts;

    int calculate_priority() const noexcept;
};

#endif
