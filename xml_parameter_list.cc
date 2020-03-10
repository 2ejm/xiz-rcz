
#include "xml_parameter_list.h"

#include "xml_string_parameter.h"
#include "xml_exception.h"

XmlParameterList::XmlParameterList ()
{ }

Glib::RefPtr <XmlParameter>
XmlParameterList::get_param (const Glib::ustring & name) const
{
    Glib::RefPtr <XmlParameter> retval;

    for (auto e : *this) {
	if (e->get_name () == name) {
	    if (retval)
		return Glib::RefPtr <XmlParameter> ();

	    retval = e;
	}
    }

    return retval;
}

Glib::RefPtr <XmlParameter>
XmlParameterList::get_first_param () const
{
    Glib::RefPtr <XmlParameter> retval=this->front();

    return retval;
}

const Glib::ustring
XmlParameterList::get_str (const Glib::ustring & name) const
{
    auto str_param = get<XmlStringParameter> (name);

    if (!str_param)
	throw XmlException ("invalid Parameter requested");

    return str_param->get_str ();
}

/**
 * /brief get string parameter, and fallback to default, if an error occurs while reading it
 */
const Glib::ustring
XmlParameterList::get_str_default (const Glib::ustring & name, const Glib::ustring & def) const
{
    Glib::ustring ret;

    try {
	ret = get_str (name);
    } catch (XmlException   &e) {
	ret = def;
    }

    return ret;
}

void
XmlParameterList::add_str_param (const Glib::ustring & name, const Glib::ustring & val)
{
    this->push_back (XmlStringParameter::create (name,val));
}

void
XmlParameterList::dump () const
{
    printf ("parameter_list dump --------------------------------\n");
    for (auto e : *this) {
	e->dump();
    }
    printf ("----------------------------------------------------\n");
}
