
#include "xml_query_generic.h"

#include <glibmm/ustring.h>

XmlQueryGeneric::XmlQueryGeneric (Glib::ustring fid,
			          XmlParameterList parameters,
				  Glib::ustring textbody)
    : XmlQuery (fid)
    , _parameters (parameters)
    , _textbody (textbody)
{ }

XmlQueryGeneric::~XmlQueryGeneric ()
{ }

Glib::RefPtr <XmlQueryGeneric>
XmlQueryGeneric::create (Glib::ustring fid,
		         XmlParameterList parameters,
			 Glib::ustring textbody)
{
    return Glib::RefPtr <XmlQueryGeneric> (new XmlQueryGeneric (fid, parameters, textbody));
}

Glib::ustring
XmlQueryGeneric::to_xml () const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose ("<function fid=\"%1\">\n", _fid);

    for (auto p : _parameters)
	ret += p->to_xml ();

    ret += _textbody;

    ret += "</function>\n";

    return ret;
}
