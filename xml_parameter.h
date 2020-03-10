
#ifndef ZIX_XML_PARAMETER_H
#define ZIX_XML_PARAMETER_H

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <glibmm/object.h>

#include <libxml++/nodes/element.h>
#include <map>

/**
 * \brief Baseclass for Parameters in xml
 *
 * Contains a mapping from xml tag to Factory.
 * We basically map xmlpp::Elements inside a <function> to
 * the Parameters which can be found in XmlFunction::_parameters
 */
class XmlParameter : public Glib::Object
{
    public:
	XmlParameter (const Glib::ustring & type, const Glib::ustring & name);

	static Glib::RefPtr<XmlParameter> create (const xmlpp::Element *en);

	typedef Glib::RefPtr<XmlParameter> (*Factory) (const xmlpp::Element *en);

	virtual void dump () = 0;
	virtual Glib::ustring to_xml () const = 0;

	static void register_factory (const Glib::ustring & name, Factory factory);
	static void init ();

	const Glib::ustring & get_name () const { return _name; }
    private:
	Glib::ustring _type;
	Glib::ustring _name;

	static std::map <Glib::ustring, Factory> factory_map;
};

#endif
