
#include "xml_result.h"

#include "log.h"
#include "xml_helpers.h"

#include <libxml++/parsers/domparser.h>

XmlResult::XmlResult ()
    : _redirected (false)
{}

XmlResult::XmlResult (int status)
    : _status (status)
    , _redirected (false)
{}

XmlResult::XmlResult (int status, const Glib::ustring & message)
    : _status (status)
    , _message (message)
    , _redirected (false)
{}

XmlResult::XmlResult (int status, const Glib::ustring & message,
                      const std::vector<Glib::ustring>& return_values)
    : _status (status)
    , _message (message)
    , _return_values (return_values)
    , _redirected (false)
{}

Glib::RefPtr <XmlResult>
XmlResult::create (int status)
{
    return Glib::RefPtr <XmlResult> (new XmlResult (status));
}

Glib::RefPtr <XmlResult>
XmlResult::create (int status, const Glib::ustring & message)
{
    return Glib::RefPtr <XmlResult> (new XmlResult (status, message));
}

Glib::RefPtr <XmlResult>
XmlResult::create (int status, const Glib::ustring & message,
                   const std::vector<Glib::ustring>& return_values)
{
    return Glib::RefPtr <XmlResult> (new XmlResult (status, message, return_values));
}

Glib::ustring
XmlResult::to_dest ()
{
    Glib::ustring res;

    if( ! _raw_result.empty() ) {
	/* This is a raw result,
	 * we need to remove the message field, and dump all other nodes into the string
	 */
	xmlpp::DomParser parser;
	parser.parse_memory (_raw_result);

	const xmlpp::Element *root = parser.get_document()->get_root_node();

	if (root == nullptr) {
	    return Glib::ustring ("failed to parse XmlResult\n");
	}

	for (auto c : root->get_children ()) {

	    /* we dont want to include <message> node in the file result
	     */
	    xmlpp::Element *en = dynamic_cast <xmlpp::Element *> (c);
	    if (en != nullptr)
		if (en->get_name() == "message")
		    continue;

	    res += node_to_string (c, 0, 0);
	}
    } else {
	for (auto&& ret : _return_values)
	    res += Glib::ustring::compose ("%1\n", ret);
	for (auto&& ret : _params)
	    res += Glib::ustring::compose ("%1\n", ret->to_xml() );
    }

    return res;
}

Glib::ustring
XmlResult::to_body ()
{
    Glib::ustring res;

    if (_message.empty () && _return_values.empty() && _params.empty() )
        return Glib::ustring();

    if (!_message.empty())
        res += Glib::ustring::compose ("<message>%1</message>\n", _message);
    for (auto&& ret : _return_values)
        res += Glib::ustring::compose ("%1\n", ret);
    for (auto&& ret : _params)
        res += Glib::ustring::compose ("%1\n", ret->to_xml() );

    return res;
}

Glib::ustring
XmlResult::to_xml ()
{
    Glib::ustring res;

    if( ! _raw_result.empty() ) {
	if (_redirected) {
	    xmlpp::DomParser parser;
	    parser.parse_memory (_raw_result);

	    xmlpp::Element *root = parser.get_document()->get_root_node();

	    if (root == nullptr) {
		return Glib::ustring ("failed to parse XmlResult\n");
	    }

	    /* search for <message> nodes, and eliminate anything but these.
	    */
	    for (auto n : root->get_children ()) {
		xmlpp::Element *en = dynamic_cast <xmlpp::Element *> (n);
		if (en != nullptr)
		    if (en->get_name() == "message")
			continue;

		/* this is not a message node... remove it
		*/
		root->remove_child (n);
	    }

	    res = node_to_string (root, 0, 0);

	    return res;
	} else {
	    return _raw_result;
	}
    }

    if (_message.empty () && _return_values.empty() && _params.empty())
        return Glib::ustring::compose ("<reply status=\"%1\"/>\n", _status);

    res += Glib::ustring::compose ("<reply status=\"%1\">\n", _status);
    if (!_redirected)
	res += to_body ();
    else // include message in redirected requests
        if (!_message.empty())
            res += Glib::ustring::compose ("<message>%1</message>\n", _message);
    res += "</reply>\n";

    return res;
}


int XmlResult::get_status()
{
    return(_status);
}


void XmlResult::dump_params()
{
    _params.dump();
}

void XmlResult::dump()
{
    printf("### Dumping XmlResult\n");
    printf("###    Status: %d\n", _status);
    printf("###    message: %s\n", _message.c_str() );
    printf("###    Return values: %d\n", (int)_return_values.size() );
    printf("###    Parameters: %d\n", (int)_params.size() );

    for (auto&& ret : _return_values)
        printf("###   Return value: %s\n", ret.c_str() );

    printf("\n");
}


void XmlResult::setRawResult(const Glib::ustring &raw_result)
{
    _raw_result=raw_result;
    return;
}


//---fin----------------------------------------------------------------------
