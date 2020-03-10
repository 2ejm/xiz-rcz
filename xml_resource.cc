
#include "xml_resource.h"
#include "xml_exception.h"

#include <cassert>


XmlResource::XmlResource (const xmlpp::Element *en)
    : XmlParameter ("resource", "resource")
{
    assert (en->get_name() == "resource");

    _id = en->get_attribute_value ("id");

    assert (! _id.empty());

    for (auto c : en->get_children ())
    {
	xmlpp::Element *en = dynamic_cast <xmlpp::Element *> (c);

	if (en != nullptr) {
	    if (en->get_name() == "parameter") {
		Glib::RefPtr <XmlResourceParameter> param = XmlResourceParameter::create (en);
		_res_params.push_back (param);
	    } else {
		/* unexpected Element seen
		 * throw XmlExceptionInvalidParameter
		 */
		throw XmlExceptionInvalidParameter (en->get_name());
	    }
	}

	/* We only care for Element Children here,
	 * ignore others
	 */
    }
}

XmlResource::~XmlResource ()
{ }


Glib::RefPtr <XmlResource>
XmlResource::create (const xmlpp::Element *en)
{
    return Glib::RefPtr <XmlResource> (new XmlResource (en));
}

Glib::RefPtr <XmlParameter>
XmlResource::param_factory (const xmlpp::Element *en)
{
    return Glib::RefPtr <XmlParameter>::cast_dynamic (create (en));
}

void
XmlResource::dump ()
{
}

Glib::ustring
XmlResource::to_xml () const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose ("<resource id=\"%1\">\n", _id);
    for (auto r : _res_params) {
	ret += r->to_xml ();
    }
    ret += "</resource>\n";

    return ret;
}
