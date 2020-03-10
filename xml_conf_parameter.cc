
#include "xml_conf_parameter.h"

#include <cassert>

XmlConfParameter::XmlConfParameter (const xmlpp::Element *en)
    : XmlParameter ("parameter", "parameter")
{
    assert (en->get_name() == "parameter");

    _id = en->get_attribute_value ("id");

    /* check wether value attribute exists.
     * its a difference, whether its empty, or doesnt exist
     */
    if (en->get_attribute ("value"))
	_has_value = true;
    else
	_has_value = false;

    _value = en->get_attribute_value ("value");

    _unit = en->get_attribute_value ("unit");
}

XmlConfParameter::~XmlConfParameter ()
{ }

Glib::RefPtr <XmlConfParameter>
XmlConfParameter::create (const xmlpp::Element *en)
{
    return Glib::RefPtr <XmlConfParameter> (new XmlConfParameter (en));
}


Glib::RefPtr <XmlParameter>
XmlConfParameter::param_factory (const xmlpp::Element *en)
{
    return Glib::RefPtr <XmlParameter>::cast_dynamic (create (en));
}


const Glib::ustring &
XmlConfParameter::get_id ()
{
    return _id;
}

const Glib::ustring &
XmlConfParameter::get_unit ()
{
    return _unit;
}

const Glib::ustring &
XmlConfParameter::get_value ()
{
    return _value;
}

bool
XmlConfParameter::has_value ()
{
    return _has_value;
}

Glib::ustring
XmlConfParameter::to_xml () const
{
    if (_has_value)
	return Glib::ustring::compose ("<parameter id=\"%1\" unit=\"%2\" value=\"%3\"/>\n", _id, _unit, _value);
    else
	return Glib::ustring::compose ("<parameter id=\"%1\" unit=\"%2\"/>\n", _id, _unit);
}


void
XmlConfParameter::dump ()
{
    printf("Config parameter: %s, value=%s\n", _id.c_str(), _value.c_str());
}

