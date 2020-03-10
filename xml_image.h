#ifndef _XML_IMAGE_H_
#define _XML_IMAGE_H_

#include <glibmm/ustring.h>

#include "xml_parameter.h"

/**
 * \brief XmlParameter representing <image> argument.
 */
class XmlImage : public XmlParameter
{
public:
    XmlImage(const xmlpp::Element *en);
    ~XmlImage();

    static Glib::RefPtr<XmlImage> create(const xmlpp::Element *en);
    static Glib::RefPtr<XmlParameter> param_factory(const xmlpp::Element *en);

    const Glib::ustring & get_signature() const
    {
        return _signature;
    }

    const Glib::ustring & get_type() const
    {
        return _type;
    }

    const Glib::ustring& get_content() const
    {
        return _content;
    }

    Glib::ustring to_xml () const;

    void dump ();

private:
    Glib::ustring _type;
    Glib::ustring _content;
    Glib::ustring _signature;
};

#endif /* _XML_MEASUREMENT_H_ */
