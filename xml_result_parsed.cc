
#include "xml_result_parsed.h"

#include "xml_helpers.h"
#include "log.h"

#include <libxml++/nodes/element.h>

#include <string>
#include <cassert>


XmlResultParsed::XmlResultParsed (const Glib::ustring & xml)
	: XmlResult ()
{
    _parser.parse_memory (xml);
    const xmlpp::Element *root = _parser.get_document()->get_root_node();

    setRawResult(xml);

    parse(root);
}


XmlResultParsed::XmlResultParsed (const unsigned char* contents, int bytes_count)
    : XmlResult ()
{
    _parser.parse_memory_raw (contents, bytes_count);
    const xmlpp::Element *root = _parser.get_document()->get_root_node();

    parse (root);
}

XmlResultParsed::XmlResultParsed (const xmlpp::Element *root)
    : XmlResult ()
{
    parse (root);
}


void XmlResultParsed::parse (const xmlpp::Element *root)
{
    assert (root->get_name() == "reply");

    for (auto attr : root->get_attributes())
    {
	if (attr->get_name () == "status") {
	    _status = std::stoi (attr->get_value ());
	} else {
	    _params.add_str_param (attr->get_name (), attr->get_value ());
	}
    }

    for (auto c : root->get_children ())
    {
	xmlpp::Element *en = dynamic_cast <xmlpp::Element *> (c);
	xmlpp::TextNode *tn = dynamic_cast <xmlpp::TextNode *> (c);

	if (en != nullptr) {
	    if (en->get_name() == "message") {
		_message = xml_element_pure_text (en);
	    } else {
		Glib::RefPtr <XmlParameter> param = XmlParameter::create (en);
		_params.push_back (param);
	    }
	} else if (tn != nullptr) {
	    _textbody += tn->get_content ();
	}

	/* We only care for Element Children, and TextNodes
	 * ignore others
	 */
    }

    return;
}


Glib::RefPtr <XmlResult>
XmlResultParsed::create (const Glib::ustring & xml)
{
    return Glib::RefPtr <XmlResult> (new XmlResultParsed (xml));
}


Glib::RefPtr <XmlResult>
XmlResultParsed::create (const unsigned char* contents, int bytes_count)
{
    return Glib::RefPtr <XmlResult> (new XmlResultParsed (contents, bytes_count));
}

Glib::RefPtr <XmlResult>
XmlResultParsed::create (const xmlpp::Element *root)
{
    return Glib::RefPtr <XmlResult> (new XmlResultParsed (root));
}
