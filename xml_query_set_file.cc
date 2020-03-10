
#include "xml_query_set_file.h"

XmlQuerySetFile::XmlQuerySetFile (Glib::RefPtr <XmlFile> file)
    : XmlQuery ("setFile")
    , _file (file)
    , _have_update (false)
{ }

XmlQuerySetFile::XmlQuerySetFile (Glib::RefPtr <XmlFile> file, const Glib::ustring & update)
    : XmlQuery ("setFile")
    , _file (file)
    , _have_update (true)
    , _update (update)
{ }

XmlQuerySetFile::XmlQuerySetFile (const Glib::ustring& type, const Glib::ustring& update, const Glib::ustring& content)
    : XmlQuery ("setFile")
    , _type (type)
    , _have_update (true)
    , _update{update}
    , _content{content}
{ }

Glib::RefPtr <XmlQuery>
XmlQuerySetFile::create (Glib::RefPtr <XmlFile> file)
{
    return Glib::RefPtr <XmlQuery> (new XmlQuerySetFile (file));
}

Glib::RefPtr <XmlQuery>
XmlQuerySetFile::create (Glib::RefPtr <XmlFile> file, const Glib::ustring & update)
{
    return Glib::RefPtr <XmlQuery> (new XmlQuerySetFile (file, update));
}


XmlQuerySetFile::~XmlQuerySetFile ()
{ }

Glib::RefPtr <XmlQuery> XmlQuerySetFile::create(
    const Glib::ustring& type, const Glib::ustring& update, const Glib::ustring& content)
{
    return Glib::RefPtr<XmlQuery>(new XmlQuerySetFile(type, update, content));
}

Glib::ustring
XmlQuerySetFile::to_xml () const
{
    Glib::ustring ret;

    if (_file) {
        ret += Glib::ustring::compose ("<function fid=\"%1\" host=\"DC\"", _fid);
	if (_have_update)
	    ret += Glib::ustring::compose (" update=\"%1\"", _update);
        ret += ">\n";
        ret += _file->to_xml ();
        ret += "</function>\n";
    } else {
        ret += Glib::ustring::compose(
            "<function fid=\"%1\" type=\"%2\" update=\"%3\">\n", _fid, _type, _update);
	ret += "<file id=\"myUpdateId\" type=\"binary\">\n";
        ret += _content;
        ret += "</file>\n";
        ret += "</function>\n";
    }

    return ret;
}
