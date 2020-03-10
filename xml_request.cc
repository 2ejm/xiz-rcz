#include <stdexcept>
#include <stdio.h>

#include "xml_function.h"
#include "xml_result_bad_request.h"
#include "xml_exception.h"
#include "zix_interface.h"
#include "accessibility_checker.h"
#include "file_handler.h"

#include "xml_request.h"
#include "procedure_step_handler.h"

int XmlRequest::req_counter = 0;

XmlRequest::XmlRequest (std::istream & is, int prio, std::weak_ptr <InterfaceConnection> ic, const Glib::ustring& interface, int tid, bool restart_proc)
    : Glib::ObjectBase (typeid (XmlRequest))
    , _parser ()
    , _prio (prio)
    , _interface_connection (ic)
    , _interface (interface)
    , _tid (tid)
    , _ts (req_counter++)
{
    _parser.parse_stream (is);

    auto conn = ic.lock();
    _function = XmlFunction::create (_parser.get_document()->get_root_node(),
                                     _interface,
                                     conn->get_filename());

    if ((_function->get_fid () == "procedure") && !restart_proc) {
	/* procedure xmls need to be saved,
	 * to enable continue through system
	 * restarts
	 */

	_parser.get_document ()->write_to_file (ZIX_PROC_SAVE_FILE_TMP);
	rename (ZIX_PROC_SAVE_FILE_TMP, ZIX_PROC_SAVE_FILE);

	/* after saving procedure, note current step (0)
	 * in current state file
	 */
	note_current_proc_step (0, interface, conn->get_resume_reply_file ());
    } else if (_function->get_fid () == "update") {
        const XmlParameterList fparams = _function->get_params ();
        Glib::ustring target;

        try {
            target = fparams.get_str ("target");

            if ((target != "DCBL") && (target != "DCF") && (target != "SICBL")) {
                /* if we have a standalone update function, also
                 * save output path to be able to write log after
                 * reboot
                 */

                /* create ZIX_UPDATE_SAVE_FILE
                 */
                FileHandler::set_file(ZIX_UPDATE_SAVE_FILE,
                    Glib::ustring::compose("<update path=\"%1\" />", conn->get_resume_reply_file ()));
            }
        } catch (XmlException & e) {
            /* continue */
        }
    }

    _prio = calculate_priority();
}

Glib::RefPtr <XmlRequest>
XmlRequest::create (std::istream & is, int prio, std::weak_ptr <InterfaceConnection> ic, const Glib::ustring& interface, int tid, bool restart_proc)
{
    return Glib::RefPtr <XmlRequest> (new XmlRequest (is, prio, ic, interface, tid, restart_proc));
}

void
XmlRequest::execute ()
{
    auto ic = _interface_connection.lock();

    // start call
    try {
	/* for an xml request, we need to set the sig_check parameter
	 */
        _current_call = _function->create_call (true);
        _current_call->finished.connect (sigc::mem_fun (*this, &XmlRequest::function_finished));
        _current_call->start_call();
    } catch (const XmlException& ex) {
        auto xml_result = XmlResultBadRequest::create(
            Glib::ustring::compose ( "Unknown function or invalid parameters: %1" , ex.what()) );
        ic->emit_result(xml_result->to_xml(), _tid);
        _current_call.reset();
        finished.emit();
    } catch (const std::exception& ex) {
        auto xml_result = XmlResultBadRequest::create(ex.what());
        ic->emit_result(xml_result->to_xml(), _tid);
        _current_call.reset();
        finished.emit();
    }
}

void
XmlRequest::function_finished (Glib::RefPtr <XmlResult> result)
{
    auto ic = _interface_connection.lock();
    auto xml_result = result->to_xml();

    /* the result is sent back over the interface_connection
     * it came from.
     */

    ic->emit_result (xml_result, _tid);

    _current_call.reset();
    finished.emit();
}

void
XmlRequest::print_node (const xmlpp::Node *node)
{
    printf ("node name: %s\n", node->get_name().c_str());

    for (auto c : node->get_children ()) {
	print_node (c);
    }
}

void
XmlRequest::print_dom ()
{
    const xmlpp::Node* node = _parser.get_document()->get_root_node();

    print_node (node);
}

int XmlRequest::prio() const noexcept
{
    return _prio;
}

int XmlRequest::ts() const noexcept
{
    return _ts;
}

Glib::ustring XmlRequest::fid() const noexcept
{
    return _function->get_fid();
}

int XmlRequest::calculate_priority() const noexcept
{
    ZixInterface inf(_interface);
    return _prio + inf.prio() + _function->prio();
}
