
#ifndef _XML_FILE_H_
#define _XML_FILE_H_

#include <glibmm/ustring.h>

#include "xml_parameter.h"

/**
 * \brief XmlParameter representing <file> argument.
 *
 * File may contain a host and id.
 */
class XmlFile : public XmlParameter
{
public:
    XmlFile(const Glib::ustring& id, const Glib::ustring& host) :
        XmlParameter("file", "file"), _id{id}, _host{host}
    {}

    XmlFile(const xmlpp::Element *en);

    ~XmlFile();

    static Glib::RefPtr<XmlFile> create(const xmlpp::Element *en);
    static Glib::RefPtr<XmlFile> create(const Glib::ustring& id, const Glib::ustring& host);
    static Glib::RefPtr<XmlParameter> param_factory(const xmlpp::Element *en);

    inline const Glib::ustring& get_host() const
    {
        return _host;
    }

    inline Glib::ustring& get_host()
    {
        return _host;
    }

    inline const Glib::ustring& get_id() const
    {
        return _id;
    }

    inline Glib::ustring& get_id()
    {
        return _id;
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

	void dump ();

	Glib::ustring to_xml () const;


private:
    Glib::ustring _id;
    Glib::ustring _host;
    Glib::ustring _type;
    Glib::ustring _content;
};

#endif /* _XML_FILE_H_ */
