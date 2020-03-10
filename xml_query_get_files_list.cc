#include "xml_query_get_files_list.h"

XmlQueryGetFilesList::XmlQueryGetFilesList()
    : XmlQuery ("getFilesList")
{ }

Glib::RefPtr<XmlQuery>
XmlQueryGetFilesList::create()
{
    return Glib::RefPtr<XmlQuery>(new XmlQueryGetFilesList());
}

XmlQueryGetFilesList::~XmlQueryGetFilesList()
{ }

Glib::ustring
XmlQueryGetFilesList::to_xml() const
{
    return Glib::ustring::compose("<function fid=\"%1\" />\n", _fid);
}
