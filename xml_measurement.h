#ifndef _XML_MEASUREMENT_H_
#define _XML_MEASUREMENT_H_

#include <glibmm/ustring.h>

#include "xml_parameter.h"

/**
 * \brief XmlParameter representing <measurement> argument.
 */
class XmlMeasurement : public XmlParameter
{
public:
    using RefPtr = Glib::RefPtr<XmlMeasurement>;

    XmlMeasurement(const xmlpp::Element *en);
    XmlMeasurement(const Glib::ustring& iid, const Glib::ustring& format);
    XmlMeasurement(const Glib::ustring& id, const Glib::ustring& iid, const Glib::ustring& format);
    ~XmlMeasurement();

    static RefPtr create(const xmlpp::Element *en);
    static RefPtr create(const Glib::ustring& iid, const Glib::ustring& format);
    static RefPtr create(const Glib::ustring& id, const Glib::ustring& iid, const Glib::ustring& format);
    static Glib::RefPtr<XmlParameter> param_factory(const xmlpp::Element *en);

    inline const Glib::ustring& get_id() const
    {
        return _id;
    }

    inline Glib::ustring& get_id()
    {
        return _id;
    }

    inline const Glib::ustring& get_iid() const
    {
        return _iid;
    }

    inline Glib::ustring& get_iid()
    {
        return _iid;
    }

    inline const Glib::ustring& get_format() const
    {
        return _format;
    }

    inline Glib::ustring& get_format()
    {
        return _format;
    }

    inline const Glib::ustring& get_type() const
    {
        return _type;
    }

    inline Glib::ustring& get_type()
    {
        return _type;
    }

    inline const Glib::ustring& get_content() const
    {
        return _content;
    }

    inline Glib::ustring& get_content()
    {
        return _content;
    }

    Glib::ustring to_xml () const;

    void dump ();

private:
    Glib::ustring _id;
    Glib::ustring _iid;
    Glib::ustring _format;
    Glib::ustring _type;
    Glib::ustring _content;
};

#endif /* _XML_MEASUREMENT_H_ */
