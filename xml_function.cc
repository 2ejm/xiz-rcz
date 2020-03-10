
#include "xml_function.h"

#include <cassert>

#include <libxml++/nodes/element.h>
#include <libxml++/parsers/domparser.h>

#include "xml_helpers.h"
#include "xml_exception.h"
#include "file_handler.h"
#include "utils.h"

#include "core_function_call.h"
#include "function_call_sigcheck.h"
#include "function_call_restricted.h"
#include "signature_check_request.h"
#include "restriction_check_request.h"
#include "accessibility_checker.h"

const XmlFunction::PrioMap XmlFunction::prio_map = {
    { "log", 0 },
    { "getFilesList", 1 },
    { "delFile", 1 },
    { "getFile", 1 },
    { "setFile", 1 },
    { "getConf", 3 },
    { "setConf", 3 },
    { "dataSync", 2 },
    { "dataOut", 1 },
    { "dataFree", 1 },
    { "getMeasurement", 1 },
    { "dataIn", 1 },
    { "getMeasurementList", 1 },
    { "getCalibration", 1 },
    { "getDefaults", 1 },
    { "getProfilesList", 1 },
    { "getProfile", 1 },
    { "getParametersList", 1 },
    { "getParameter", 1 },
    { "getTemplatesList", 1 },
    { "getTemplate", 1 },
    { "setMeasurementsList", 1 },
    { "setMeasurement", 1 },
    { "setCalibration", 1 },
    { "setDefaults", 1 },
    { "setProfilesList", 1 },
    { "setProfile", 1 },
    { "setParameter", 1 },
    { "setTemplatesList", 1 },
    { "setTemplate", 1 },
    { "delMeasurement", 1 },
    { "delProfile", 1 },
    { "delTemplate", 1 },
    { "guiIO", 3 },
    { "procedure", 1 },
};

XmlFunction::XmlFunction (const xmlpp::Element *elem,
                          const Glib::ustring &interface,
                          const Glib::ustring &filename)
    : XmlParameter ("function", "function")
    , _elem (elem)
    , _interface (interface)
    , _filename (filename)
{
    assert (elem->get_name() == "function");

    for (auto attr : elem->get_attributes())
    {
	if (attr->get_name () == "fid") {
	    _fid = attr->get_value ();
	} else if (attr->get_name () == "src") {
	    _src = attr->get_value ();
            _parameters.add_str_param (attr->get_name (), _src);
	} else if (attr->get_name () == "dest") {
	    _dest = attr->get_value ();
        /*
         * dest has a different meaning for dataOut. That's why
         * dest has to be added to _parameters as well.
         */
        _parameters.add_str_param (attr->get_name (), _dest);
	} else {
	    _parameters.add_str_param (attr->get_name (), attr->get_value ());
	}
    }

    for (auto c : elem->get_children ())
    {
	const xmlpp::Element *en = dynamic_cast <xmlpp::Element *> (c);
	const xmlpp::TextNode *tn = dynamic_cast <xmlpp::TextNode *> (c);

	if (en != nullptr) {
	    if (en->get_name() == "description") {
		_description = XmlDescription::create(en);
	    } else if (en->get_name() == "restriction") {
                _restrictions.emplace_back(XmlRestriction::create(en));
            } else if (en->get_name() == "signature") {
                _signature = XmlSignature::create(en);
            } else if (en->get_name() == "function") {
		/* Function Parameter needs interface in constructor
		 */
		auto func = Glib::RefPtr <XmlParameter>::cast_dynamic (create (en, _interface, _filename));
		_parameters.push_back (func);
            } else {
		Glib::RefPtr <XmlParameter> param = XmlParameter::create (en);
		_parameters.push_back (param);
	    }
	} else if (tn != nullptr) {
            _textbody += tn->get_content ();
        }

	/* We only care for Element Children, and TextNodes
	 * ignore others
	 */
    }
}

Glib::RefPtr <XmlFunction>
XmlFunction::create (const xmlpp::Element *en, const Glib::ustring &interface,
                     const Glib::ustring &filename)
{
    return Glib::RefPtr <XmlFunction> (new XmlFunction (en, interface, filename));
}

Glib::RefPtr <XmlFunction>
XmlFunction::load_src ()
{
    Glib::ustring xml_string;

    xml_string  = Glib::ustring::compose ("<function fid=\"%1\">\n", _fid);
    xml_string += FileHandler::get_file (_src);
    xml_string += "</function>";

    _file_parser.parse_memory (xml_string);

    xmlpp::Node *xn = _file_parser.get_document()->get_root_node();
    xmlpp::Element *en = dynamic_cast <xmlpp::Element *> (xn);

    assert (en);

    return Glib::RefPtr <XmlFunction> (new XmlFunction (en, _interface, _filename));
}

Glib::RefPtr <FunctionCall>
XmlFunction::create_call (bool sig_check)
{
    Glib::RefPtr <XmlFunction> file_func;
    const xmlpp::Element * wrappo = _elem;

    if (!_src.empty() && _fid != "dataIn") {
	/* we have a src tag setup.
	 * need to parse xml body from file
	 */
	file_func = load_src ();

	/* save an eventual dest parameter
	 */
	Glib::RefPtr <XmlParameter> dest_param = _parameters.get<XmlParameter>("dest");

	/* we keep the description from the original xml,
	 * only _parameters and _textbody is taken from
	 * file_func.
	 */

	// XXX: maybe we want to save the old parameters here...
	//	would be easy.. dont know, yet.
	_parameters = file_func->_parameters;
	_textbody = file_func->_textbody;

	/* we override the wrappo element,
	 * which will get presented to the DC
	 */
	wrappo = file_func->_elem;

	/* potentially add saved dest parameter
	 */
	if (dest_param)
	    _parameters.push_back (dest_param);
    }

    /* do access check first
     */
    if (!AccessibilityChecker().can_be_accessed(_interface, _fid)) {
	throw XmlExceptionNoAccess (_fid, _interface);
    }

    /* Create the function call
     */
    auto call = CoreFunctionCall::create (_fid,
					  _parameters,
					  _description,
					  _textbody,
					  wrappo,
					  _interface,
                                          _filename);

    /* maybe wrap in signature check
     */
    if (sig_check) {
	Glib::ustring sig;

	if (_signature) {
	    sig = _signature->signature ();
	}

	auto scr = SignatureCheckRequest::create (sig,
						  _interface,
						  _fid,
						  _elem);

	call = FunctionCallSigCheck::create (call, scr, _interface);
    }

    /* maybe wrap in restriction check
     */
    if (_restrictions.size () > 0) {
	auto rcr = RestrictionCheckRequest::create (_restrictions, _fid);
	call = FunctionCallRestricted::create (call, rcr, _interface);
    }

    return call;
}

void
XmlFunction::dump ()
{
    printf ("XmlFunction::dump ----------------------------------\n");
    printf ("_fid:  %s\n", _fid.c_str ());
    printf ("_dest: %s\n", _dest.c_str ());
    printf ("_parameters:\n");

    _parameters.dump ();

    if (_description)
	_description->dump ();
    else
	printf ("no desription\n");


    printf ("----------------------------------------------------\n");
}

Glib::ustring
XmlFunction::to_xml () const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose ("<function fid=\"%1\">\n", _fid);
    ret += "</function>\n";

    return ret;
}
