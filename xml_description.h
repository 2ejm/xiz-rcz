
#ifndef ZIX_XML_DESCRIPTION_H
#define ZIX_XML_DESCRIPTION_H

#include "xml_parameter.h"

/**
 * \brief XmlParameter representing <description> argument
 *
 * descriptions are generally for different languages.
 * There is also a default language.
 *
 * \note unsure, whether we just send description to DC
 */
class XmlDescription : public XmlParameter
{
    public:
	XmlDescription (const xmlpp::Element *en);
	~XmlDescription ();

	static Glib::RefPtr<XmlDescription> create (const xmlpp::Element *en);
	static Glib::RefPtr<XmlParameter> param_factory (const xmlpp::Element *en);

	const Glib::ustring & get_description (const Glib::ustring & lang);

	void dump ();
	Glib::ustring to_xml () const;
	Glib::ustring to_body () const;

    private:
	std::map <Glib::ustring, Glib::ustring> _description;
	Glib::ustring _default_lang;
};
#endif
