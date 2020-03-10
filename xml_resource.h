
#ifndef ZIX_XML_RESOURCE_H
#define ZIX_XML_RESOURCE_H

#include "xml_parameter.h"
#include "xml_resource_parameter.h"

/**
 * \brief XmlParameter representing <resource> argument
 *
 * <resource> is used in the Config API
 */
class XmlResource : public XmlParameter
{
    public:
	XmlResource (const xmlpp::Element *en);
	~XmlResource ();

	static Glib::RefPtr<XmlResource> create (const xmlpp::Element *en);
	static Glib::RefPtr<XmlParameter> param_factory (const xmlpp::Element *en);
    const Glib::ustring &get_id()
    {
        return(_id);
    }
    const std::list <Glib::RefPtr <XmlResourceParameter> > &get_resource_params()
    {
        return(_res_params);
    }


	 void dump ();
	 Glib::ustring to_xml () const;

    private:
	Glib::ustring _id;
	std::list <Glib::RefPtr <XmlResourceParameter> > _res_params;
};
#endif
