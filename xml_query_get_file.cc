#include "xml_query_get_file.h"

XmlQueryGetFile::XmlQueryGetFile(const Glib::ustring& file)
    : XmlQuery ("getFile")
    , _file (file)
{ }

Glib::RefPtr<XmlQuery>
XmlQueryGetFile::create(const Glib::ustring& file)
{
    return Glib::RefPtr<XmlQuery>(new XmlQueryGetFile(file));
}

XmlQueryGetFile::~XmlQueryGetFile()
{ }

Glib::ustring
XmlQueryGetFile::to_xml() const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose("<function fid=\"%1\">\n", _fid);
    if(!_file.empty())
        ret += Glib::ustring::compose("<file id=\"%1\" />\n", _file);
    ret += "</function>\n";

    return ret;
}
