

#ifndef ZIX_XML_RESOURCE_PARAMETER_H
#define ZIX_XML_RESOURCE_PARAMETER_H

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <glibmm/object.h>

#include <libxml++/nodes/element.h>

/**
 * \brief XmlParameter representing <resource> argument
 *
 * <resource> is used in the Config API
 */
class XmlResourceParameter : public Glib::Object
{
    public:
	XmlResourceParameter (xmlpp::Element *en);
	~XmlResourceParameter ();

	static Glib::RefPtr<XmlResourceParameter> create (xmlpp::Element *en);

	const Glib::ustring & get_id ();
	const Glib::ustring & get_unit ();
	const Glib::ustring & get_value ();

	Glib::ustring to_xml () const;

    private:
	Glib::ustring _id;
	Glib::ustring _unit;
	Glib::ustring _value;
};
#endif
