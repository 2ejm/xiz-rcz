
#include "xml_query_pass.h"

#include <glibmm/ustring.h>

XmlQueryPass::XmlQueryPass (Glib::ustring fid, Glib::ustring xml)
    : XmlQuery (fid)
    , _xml (xml)
{ }

XmlQueryPass::~XmlQueryPass ()
{ }

Glib::RefPtr <XmlQueryPass>
XmlQueryPass::create (Glib::ustring fid, Glib::ustring xml)
{
    return Glib::RefPtr <XmlQueryPass> (new XmlQueryPass (fid, xml));
}

Glib::ustring
XmlQueryPass::to_xml () const
{
    return _xml;
}
