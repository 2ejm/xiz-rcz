
#include "xml_resource_parameter.h"

#include <cassert>

XmlResourceParameter::XmlResourceParameter (xmlpp::Element *en)
{
    assert (en->get_name() == "parameter");

    _id = en->get_attribute_value ("id");
    _value = en->get_attribute_value ("value");
    _unit = en->get_attribute_value ("unit");
}

XmlResourceParameter::~XmlResourceParameter ()
{ }

Glib::RefPtr <XmlResourceParameter>
XmlResourceParameter::create (xmlpp::Element *en)
{
    return Glib::RefPtr <XmlResourceParameter> (new XmlResourceParameter (en));
}

const Glib::ustring &
XmlResourceParameter::get_id ()
{
    return _id;
}

const Glib::ustring &
XmlResourceParameter::get_unit ()
{
    return _unit;
}

const Glib::ustring &
XmlResourceParameter::get_value ()
{
    return _value;
}

Glib::ustring
XmlResourceParameter::to_xml () const
{
    return Glib::ustring::compose ("<parameter id=\"%1\" unit=\"%2\" value=\"%3\"/>\n", _id, _unit, _value);
	
}

