#include <iostream>
#include <cassert>

#include "xml_file.h"
#include "xml_helpers.h"

XmlFile::XmlFile (const xmlpp::Element *en)
    : XmlParameter ("file", "file")
{
    assert (en);
    assert (en->get_name() == "file");

    _id      = en->get_attribute_value("id");
    _host    = en->get_attribute_value("host");
    _type    = en->get_attribute_value("type");
    _content = xml_element_pure_text(en);
}

XmlFile::~XmlFile ()
{ }

Glib::RefPtr<XmlFile>
XmlFile::create(const xmlpp::Element *en)
{
    return Glib::RefPtr<XmlFile>(new XmlFile(en));
}

Glib::RefPtr<XmlFile>
XmlFile::create(const Glib::ustring& id, const Glib::ustring& host)
{
    return Glib::RefPtr<XmlFile>(new XmlFile(id, host));
}

Glib::RefPtr<XmlParameter>
XmlFile::param_factory(const xmlpp::Element *en)
{
    return Glib::RefPtr<XmlParameter>::cast_dynamic(create(en));
}

void
XmlFile::dump ()
{
    std::cout << "_host: "    << _host    << std::endl;
    std::cout << "_id: "      << _id      << std::endl;
    std::cout << "_type: "    << _type    << std::endl;
    std::cout << "_content: " << _content << std::endl;
}

Glib::ustring
XmlFile::to_xml () const
{
    Glib::ustring ret;

    ret += "<file ";
    if (!_host.empty())
        ret += Glib::ustring::compose("host=\"%1\" ", _host);
    if (!_id.empty())
        ret += Glib::ustring::compose("id=\"%1\" ", _id);
    if (!_type.empty())
        ret += Glib::ustring::compose("type=\"%1\" ", _type);
    if (!_content.empty())
        ret += Glib::ustring::compose(">\n%1</file>\n", _content);
    else
        ret += "/>\n";

    return ret;
}
