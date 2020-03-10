#ifndef ZIX_XML_CONF_PARAMETER_H
#define ZIX_XML_CONF_PARAMETER_H

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <glibmm/object.h>

#include <libxml++/nodes/element.h>

#include "xml_parameter.h"

/**
 * \brief XmlParameter representing <resource> argument
 *
 * <resource> is used in the Config API
 */
class XmlConfParameter : public XmlParameter
{
    public:
    XmlConfParameter (const xmlpp::Element *en);
    ~XmlConfParameter ();

    static Glib::RefPtr<XmlConfParameter> create (const xmlpp::Element *en);
    static Glib::RefPtr <XmlParameter> param_factory (const xmlpp::Element *en);

	const Glib::ustring & get_id ();
	const Glib::ustring & get_unit ();
	const Glib::ustring & get_value ();
	bool has_value ();

    virtual void dump ();

	Glib::ustring to_xml () const;

    private:
	Glib::ustring _id;
	Glib::ustring _unit;
	Glib::ustring _value;
	bool _has_value;
};
#endif
