
#include "xml_processor.h"

#include "xml_result_bad_request.h"
#include "utils.h"
#include "file_handler.h"
#include "procedure_step_handler.h"
#include "file_interface_connection.h"
#include "accessibility_checker.h"
#include "zix_interface.h"
#include "xml_helpers.h"

#include <fstream>

XmlProcessor::XmlProcessor ()
    : Glib::ObjectBase (typeid (XmlProcessor)),
    _prio_queue([] (const ReqPtr& r1, const ReqPtr& r2) -> bool
                {
                    if (r1->prio() == r2->prio())
                        return r1->ts() > r2->ts();
                    return r1->prio() < r2->prio();
                }),
    _high_prio_queue([] (const ReqPtr& r1, const ReqPtr& r2) -> bool
                    {
                        if (r1->prio() == r2->prio())
                            return r1->ts() > r2->ts();
                        return r1->prio() < r2->prio();
                    })
{}

Glib::RefPtr <XmlProcessor>
XmlProcessor::create ()
{
    return Glib::RefPtr <XmlProcessor> (new XmlProcessor ());
}

void
XmlProcessor::parse_stream (std::istream & is, int tid, int prio, std::weak_ptr <InterfaceConnection> ic, const Glib::ustring& interface, bool restart_proc)
{
    Glib::RefPtr <XmlRequest> xml_req;
    auto conn = ic.lock();

    try {
	xml_req = XmlRequest::create (is, prio, ic, interface, tid, restart_proc);
    } catch (const std::exception& e) {
	/* Failed to create the Request,
	 * this is generally a Parsererror
	 */

	auto result = XmlResultBadRequest::create (e.what());
	conn->emit_result (result->to_xml (), tid);
	return;
    }

    /*
     * Execution of tasks:
     *  - FIFO based on function and interface priority
     *
     * There are two exceptions to the rule:
     *  1. A task is running *and* a task from DC arrives. This tasks has
     *     priority should be executed immediatly.
     *  2. A guiIO call arrives when something else is running. The scripts e.g.
     *     called by dataSync, dataOut, ... call guiIO. Therefore, these have to
     *     executed as well. Otherwise we end up in a deadlock.
     *
     * According to Dr. Buehrle (via mail) the scripts only call guiIO.
     */

    /*
     * Check for exception 2.
     */
    if ((_current_request || _current_high_prio_request) &&
        xml_req->fid() == "guiIO") {
        _gui_queue.push(xml_req);
        if (!_current_gui_request)
            execute_next_gui_request();
        return;
    }

    /*
     * Check for exception 1.
     */
    if (_current_request && interface == STR_ZIXINF_IPC) {
        _high_prio_queue.push(xml_req);
        if (!_current_high_prio_request)
            execute_next_high_prio_request();
        return;
    }

    /*
     * Common case: Execute tasks in FIFO order.
     */
    _prio_queue.push (xml_req);
    if (!_current_request)
	execute_top_request ();
}

void
XmlProcessor::execute_next_high_prio_request ()
{
    PRINT_DEBUG("execute_next_high_prio_request()");
    if (_high_prio_queue.empty())
        return;

    _current_high_prio_request = _high_prio_queue.top();
    _high_prio_queue.pop();
    _current_high_prio_request->finished.connect(
        sigc::mem_fun(*this, &XmlProcessor::high_prio_request_finished));
    _current_high_prio_request->execute();
}

void
XmlProcessor::execute_next_gui_request ()
{
    PRINT_DEBUG("execute_gui_prio_request()");
    if (_gui_queue.empty())
        return;

    _current_gui_request = _gui_queue.top();
    _gui_queue.pop();
    _current_gui_request->finished.connect(
        sigc::mem_fun(*this, &XmlProcessor::gui_request_finished));
    _current_gui_request->execute();
}

void
XmlProcessor::execute_top_request ()
{
    PRINT_DEBUG("execute_top_request()");
    if (_prio_queue.empty())
	return;

    _current_request = _prio_queue.top ();

    _prio_queue.pop ();

    _current_request->finished.connect (sigc::mem_fun (*this, &XmlProcessor::request_finished));
    _current_request->execute ();
}

void
XmlProcessor::request_finished ()
{
    _current_request.reset();

    if (_prio_queue.size() > 0)
	execute_top_request ();
}

void
XmlProcessor::high_prio_request_finished ()
{
    _current_high_prio_request.reset();

    if (_high_prio_queue.size() > 0)
        execute_next_high_prio_request();
}

void
XmlProcessor::gui_request_finished ()
{
    _current_gui_request.reset();

    if (_gui_queue.size() > 0)
        execute_next_gui_request();
}

void
XmlProcessor::init()
{
    if (FileHandler::file_exists (ZIX_UPDATE_SAVE_FILE)) {
        try {
            if (FileHandler::file_exists (ZIX_UPDATE_RESULT_FILE)) {
                PRINT_DEBUG (ZIX_UPDATE_RESULT_FILE << " exists");
                xmlpp::DomParser file_parser;

                file_parser.parse_file (ZIX_UPDATE_RESULT_FILE);

                xmlpp::Document *ur_doc = file_parser.get_document ();
                xmlpp::Element  *ur_e   = ur_doc->get_root_node ();
                Glib::ustring result_string =  element_to_string(ur_e, 0, 0);

                file_parser.parse_file (ZIX_UPDATE_SAVE_FILE);
                ur_doc = file_parser.get_document ();
                Glib::ustring path = ur_doc->get_root_node()->get_attribute_value ("path");
                FileHandler::set_file(path, result_string);

                FileHandler::del_file (ZIX_UPDATE_RESULT_FILE);
                FileHandler::del_file (ZIX_UPDATE_SAVE_FILE);
                PRINT_DEBUG ("removed file " << ZIX_UPDATE_RESULT_FILE);
            } else {
                FileHandler::del_file (ZIX_UPDATE_SAVE_FILE);
                PRINT_DEBUG ("could not find " << ZIX_UPDATE_RESULT_FILE);
            }
        } catch (const std::exception& ex) {
            PRINT_ERROR("Failed to handle update file handling: " << ex.what());
        }
    }

    if (get_current_proc_step () >= 0) {

	if (FileHandler::file_exists (ZIX_PROC_SAVE_FILE)) {
	    std::ifstream save_xml (ZIX_PROC_SAVE_FILE);
	    //XXX: need to create some sort of tmp InterfaceConnection,
	    //     because we dont know where to put the result, yet...
	    _procedure_connection =
                FileInterfaceConnection::create_reply_connection (get_current_result_filename ());

	    parse_stream (save_xml, 0, 0, _procedure_connection, get_current_interface (), true);

	} else {
	    // XXX: its an error, if the ZIX_PROC_SAVE_FILE does not exist
	    //      however, we cant do much more than resetting the current proc_step
	    note_current_proc_step (-1);
	}
    }
}
