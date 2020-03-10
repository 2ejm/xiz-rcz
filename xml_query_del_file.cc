#include "xml_query_del_file.h"

XmlQueryDelFile::XmlQueryDelFile(const Glib::ustring& file)
    : XmlQuery ("delFile")
    , _file (file)
{ }

Glib::RefPtr<XmlQuery>
XmlQueryDelFile::create(const Glib::ustring& file)
{
    return Glib::RefPtr<XmlQuery>(new XmlQueryDelFile (file));
}

XmlQueryDelFile::~XmlQueryDelFile()
{ }

Glib::ustring
XmlQueryDelFile::to_xml() const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose("<function fid=\"%1\">\n", _fid);
    ret += Glib::ustring::compose("<file id=\"%1\" />\n", _file);
    ret += "</function>\n";

    return ret;
}
