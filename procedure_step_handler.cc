
#include "procedure_step_handler.h"

#include "file_handler.h"
#include "xml_result.h"
#include "xml_result_parsed.h"
#include "utils.h"

#include <libxml++/parsers/domparser.h>
#include <libxml++/document.h>
#include <libxml++/nodes/element.h>

#include <unistd.h>
#include <cassert>

Glib::ustring current_interface;
std::list <Glib::RefPtr <XmlResult> > current_result_list;
Glib::ustring current_result_filename;
bool proc_in_progress;

int
get_current_proc_step ()
{
    try {
	xmlpp::DomParser file_parser;
	file_parser.parse_file (ZIX_PROC_STEP_FILE);

	xmlpp::Document *doc = file_parser.get_document ();
	xmlpp::Element  *e   = doc->get_root_node ();
	int curr_step;

	/* save current interface every time, when reading ZIX_PROC_STEP_FILE
	 */
	current_interface = e->get_attribute_value ("interface");

	/* also reget current_result_filename
	 */
	current_result_filename = e->get_attribute_value ("result_filename");

	/* rescan result list
	 */
	current_result_list.clear ();
	for (auto r : e->get_children ("reply")) {
	    const xmlpp::Element *e = dynamic_cast <const xmlpp::Element *> (r);
	    if (e) {
		current_result_list.push_back (XmlResultParsed::create (e));
	    }
	}
	Glib::ustring step_str = e->get_attribute_value ("step");

	/* empty string means, that we dont execute a procedure
	 * currently
	 */
	if (step_str.empty()) {
            proc_in_progress = false;
	    return -1;
        }

	curr_step = atoi (step_str.c_str ());
	PRINT_DEBUG ("get_current_proc_step curr_step " << curr_step);

	/* after reading in the ZIX_PROC_STEP_FILE
	 * we check for a ZIX_UPDATE_RESULT_FILE
	 * and incorporate that into the current state
	 */
	if (FileHandler::file_exists (ZIX_UPDATE_RESULT_FILE)) {
	    PRINT_DEBUG (ZIX_UPDATE_RESULT_FILE << " exists");
	    xmlpp::DomParser ur_parser;
	    file_parser.parse_file (ZIX_UPDATE_RESULT_FILE);

	    xmlpp::Document *ur_doc = file_parser.get_document ();
	    xmlpp::Element  *ur_e   = ur_doc->get_root_node ();

	    current_result_list.push_back (XmlResultParsed::create (ur_e));
	    curr_step += 1;

	    note_current_proc_step (curr_step, 0);
	    FileHandler::del_file (ZIX_UPDATE_RESULT_FILE);
	    PRINT_DEBUG ("removed file " << ZIX_UPDATE_RESULT_FILE);
	}

	if (curr_step) {
	    PRINT_DEBUG ("get_current_proc_step returns " << curr_step);
            proc_in_progress = true;
	    return curr_step;
	}


    } catch (std::exception &e) {
	/* when something happens, we just return -1
	 * so, we just fall through
	 */
	PRINT_ERROR ("get_current_proc_step got exception: " << e.what ());
    }

    proc_in_progress = false;
    return -1;
}

int
get_current_retry ()
{
	xmlpp::DomParser file_parser;
	file_parser.parse_file (ZIX_PROC_STEP_FILE);

	xmlpp::Document *doc = file_parser.get_document ();
	xmlpp::Element  *e   = doc->get_root_node ();

	/* save current interface every time, when reding ZIX_PROC_STEP_FILE
	 */
	current_interface = e->get_attribute_value ("interface");

	Glib::ustring step_str = e->get_attribute_value ("retry");
	if (! step_str.empty ()) {
	    return atoi (step_str.c_str());
	}

	return -1;
}


const Glib::ustring &
get_current_interface ()
{
    return current_interface;
}

const Glib::ustring &
get_current_result_filename ()
{
    return current_result_filename;
}

const std::list <Glib::RefPtr <XmlResult> > &
get_current_result_list ()
{
    return current_result_list;
}

bool procedure_in_progress()
{
    return proc_in_progress;
}

void
note_current_proc_step (int step, int retry)
{
    PRINT_DEBUG ("note_current_proc_step () step=" << step);
    assert (! current_interface.empty ());

    /* step -1 means, that the procedure has completed
     * unlink xml file
     */
    if (step == -1) {
        proc_in_progress = false;
	unlink (ZIX_PROC_SAVE_FILE);
	current_interface = "";
	current_result_list.clear ();
    }

    Glib::ustring proc_step_xml;
    proc_step_xml = Glib::ustring::compose ("<procedure step=\"%1\" retry=\"%2\" interface=\"%3\" result_filename=\"%4\">\n",
					    step,
					    retry,
					    current_interface,
					    current_result_filename);

    for (auto r : current_result_list) {
	proc_step_xml += r->to_xml ();
    }
    proc_step_xml += "</procedure>";
    FileHandler::set_file (ZIX_PROC_STEP_FILE_TMP, proc_step_xml);
    rename (ZIX_PROC_STEP_FILE_TMP, ZIX_PROC_STEP_FILE);

    proc_in_progress = true;
}

void
note_current_proc_step (int step, const Glib::ustring & interface, const Glib::ustring & result_filename)
{
    current_interface = interface;
    current_result_filename = result_filename;

    note_current_proc_step (step, 0);
}

void
note_current_proc_step (int step, const std::list <Glib::RefPtr <XmlResult> > & result_list)
{
    current_result_list = result_list;

    note_current_proc_step (step, 0);
}

void
note_current_proc_step (int step, int retry, const std::list <Glib::RefPtr <XmlResult> > & result_list)
{
    current_result_list = result_list;

    note_current_proc_step (step, retry);
}

bool
check_exec_exceeded(const Glib::ustring& id, const int exec)
{
    int executed = 0;
    xmlpp::DomParser parser;
    if (FileHandler::file_exists(ZIX_EXEC_ATTR_FILE)){
        parser.parse_file(ZIX_EXEC_ATTR_FILE);
        xmlpp::Node *root = parser.get_document()->get_root_node();

        auto nodeSet = root->find(
            Glib::ustring::compose("//procedure[@id='%1']", id));

        if (nodeSet.size() > 1) {
            PRINT_ERROR("More than id found. Something is wrong not executing procedure");
            return true;
        }

        if (nodeSet.size() == 0) {
            /* node id not found
             * not yet in list, adding it to list
             */
	    nodeSet = root-> find("//procedures");
	    if (nodeSet.size() < 1)
                EXCEPTION("Faild to find parantnode procedures");

            auto *node = nodeSet.front()->add_child("procedure");
            if (!node)
                EXCEPTION("Failed to add procedure node");

            auto *elem = dynamic_cast<xmlpp::Element *>(node);
            if (!elem)
                EXCEPTION("Failed to add procedure node ");

            elem->set_attribute("id", id);
            elem->set_attribute("executed", std::to_string(executed));
            parser.get_document()->write_to_file_formatted(ZIX_EXEC_ATTR_FILE);
            return false;
        }
        /* compare exec and executed
         */
        auto elem = dynamic_cast<xmlpp::Element *>(nodeSet.front());
        executed = std::stoi(elem->get_attribute_value("executed"));

        if (executed < exec){
            return false;
        }
	/* exec > executed
	 * not executing again
	 */
        return true;
    }
    else {
	create_procedure_exec_file(id);
	return false;
    }
}
void create_procedure_exec_file(const Glib::ustring& id)
{
    Glib::ustring node = Glib::ustring::compose("<procedures><procedure id='%1' executed='0'/>", id);
    node += "</procedures>";
    FileHandler::set_file(ZIX_EXEC_ATTR_FILE, node);
}
bool increment_exec_attribute(const Glib::ustring& id)
{
    xmlpp::DomParser parser;
    if (FileHandler::file_exists(ZIX_EXEC_ATTR_FILE)){
        parser.parse_file(ZIX_EXEC_ATTR_FILE);
        xmlpp::Node *root = parser.get_document()->get_root_node();

        auto nodeSet = root->find(
            Glib::ustring::compose("//procedure[@id='%1']", id));

        if (nodeSet.size() > 1) {
            PRINT_ERROR("More than one id found. Something is wrong not executing procedure");
            return false;
        }
        if (nodeSet.size() == 0) {
            PRINT_ERROR("Node not found. Something is wrong procedure id should already be listed.");
            return false;
        }

        auto elem = dynamic_cast<xmlpp::Element *>(nodeSet.front());
        int executed = std::stoi(elem->get_attribute_value("executed"));

        executed++;
        elem->set_attribute("executed", std::to_string(executed));
        parser.get_document()->write_to_file_formatted(ZIX_EXEC_ATTR_FILE);
        return true;
    }

    PRINT_ERROR(ZIX_EXEC_ATTR_FILE " does not exist.");
    return false;

}
