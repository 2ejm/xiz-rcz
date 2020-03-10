
#ifndef ZIX_XML_PARAMETER_LIST_H
#define ZIX_XML_PARAMETER_LIST_H

#include <list>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include "xml_parameter.h"

/**
 * \brief List of Parameters XmlFunction::_parameters
 *
 * XmlParameterList is also passed to the CoreFunctionCall, when
 * its created.
 */
class XmlParameterList : public std::list <Glib::RefPtr <XmlParameter> >
{
    public:
	XmlParameterList ();

	Glib::RefPtr <XmlParameter> get_param (const Glib::ustring & name) const;
    Glib::RefPtr <XmlParameter> get_first_param () const;

	template <class T>
	Glib::RefPtr <T> get (const Glib::ustring & name) const {
	    Glib::RefPtr <XmlParameter> p = get_param (name);
	    return Glib::RefPtr <T>::cast_dynamic (p);
	}

    template <class T>
    Glib::RefPtr <T> get_first () const {
        Glib::RefPtr <XmlParameter> p = get_first_param ();
        return Glib::RefPtr <T>::cast_dynamic (p);
    }

	template <class T>
	std::list <Glib::RefPtr <T> > get_all (const Glib::ustring & name) const
	{
	    std::list <Glib::RefPtr <T> > retval;

	    for (auto e : *this) {
		if (e->get_name () == name) {
		    retval.push_back (Glib::RefPtr <T>::cast_dynamic (e));
		}
	    }

	    return retval;
	}

	const Glib::ustring get_str (const Glib::ustring & name) const ;
	const Glib::ustring get_str_default (const Glib::ustring & name, const Glib::ustring & def) const;

	void add_str_param (const Glib::ustring & name, const Glib::ustring & val);

	void dump () const;
};

#endif
