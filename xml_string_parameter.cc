
#include "xml_string_parameter.h"

XmlStringParameter::XmlStringParameter (const Glib::ustring & name, const Glib::ustring & val)
    : XmlParameter ("string", name)
    , _val (val)
{ }

Glib::RefPtr <XmlStringParameter>
XmlStringParameter::create (const Glib::ustring & name, const Glib::ustring & val)
{
    return Glib::RefPtr <XmlStringParameter> (new XmlStringParameter (name, val));
}

const Glib::ustring &
XmlStringParameter::get_str ()
{
    return _val;
}

void
XmlStringParameter::dump ()
{
    printf ("string param: name=%s val=%s\n", get_name().c_str(), _val.c_str());
}

Glib::ustring
XmlStringParameter::to_xml () const
{
    return Glib::ustring::compose ("<%1>%2</%3>\n", get_name (), _val, get_name ());
}
