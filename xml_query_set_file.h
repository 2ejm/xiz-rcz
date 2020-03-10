#ifndef ZIX_XML_QUERY_SET_FILE_H
#define ZIX_XML_QUERY_SET_FILE_H

#include "xml_query.h"
#include "xml_file.h"

class XmlQuerySetFile : public XmlQuery
{
public:
    XmlQuerySetFile (Glib::RefPtr <XmlFile> file);
    XmlQuerySetFile (Glib::RefPtr <XmlFile> file, const Glib::ustring & update);
    XmlQuerySetFile (const Glib::ustring& type, const Glib::ustring& update, const Glib::ustring& content);

    ~XmlQuerySetFile ();

    static Glib::RefPtr <XmlQuery> create (Glib::RefPtr <XmlFile> file);
    static Glib::RefPtr <XmlQuery> create (Glib::RefPtr <XmlFile> file, const Glib::ustring & update);
    static Glib::RefPtr <XmlQuery> create(
        const Glib::ustring& type, const Glib::ustring& update, const Glib::ustring& content);

    Glib::ustring to_xml () const;

private:
    Glib::RefPtr <XmlFile> _file;

    Glib::ustring _type;
    bool _have_update;
    Glib::ustring _update;
    Glib::ustring _content;
};

#endif
