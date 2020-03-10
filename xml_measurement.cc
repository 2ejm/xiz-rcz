#include <iostream>
#include <cassert>

#include "xml_measurement.h"
#include "xml_helpers.h"

XmlMeasurement::XmlMeasurement (const xmlpp::Element *en)
    : XmlParameter("measurement", "measurement")
{
    assert(en);
    assert(en->get_name() == "measurement");

    _id      = en->get_attribute_value("id");
    _iid     = en->get_attribute_value("iid");
    _format  = en->get_attribute_value("format");
    _type    = en->get_attribute_value("type");
    _content = element_to_string(en, 0, 0);
}

XmlMeasurement::XmlMeasurement(const Glib::ustring& iid, const Glib::ustring& format) :
    XmlParameter("measurement", "measurement"), _iid{iid}, _format{format}
{}

XmlMeasurement::XmlMeasurement(const Glib::ustring& id, const Glib::ustring& iid, const Glib::ustring& format) :
    XmlParameter("measurement", "measurement"), _id{id}, _iid{iid}, _format{format}
{}

XmlMeasurement::~XmlMeasurement ()
{ }

Glib::RefPtr <XmlMeasurement>
XmlMeasurement::create(const xmlpp::Element *en)
{
    return Glib::RefPtr<XmlMeasurement>(new XmlMeasurement(en));
}

Glib::RefPtr<XmlMeasurement>
XmlMeasurement::create(const Glib::ustring& iid, const Glib::ustring& format)
{
    return Glib::RefPtr<XmlMeasurement>(new XmlMeasurement(iid, format));
}

Glib::RefPtr<XmlMeasurement>
XmlMeasurement::create(const Glib::ustring& id, const Glib::ustring& iid, const Glib::ustring& format)
{
    return Glib::RefPtr<XmlMeasurement>(new XmlMeasurement(id, iid, format));
}

Glib::RefPtr<XmlParameter>
XmlMeasurement::param_factory(const xmlpp::Element *en)
{
    return Glib::RefPtr<XmlParameter>::cast_dynamic(create(en));
}

void XmlMeasurement::dump()
{
    std::cout << "_id: "      << _id      << std::endl;
    std::cout << "_iid: "     << _iid     << std::endl;
    std::cout << "_format: "  << _format  << std::endl;
    std::cout << "_type: "    << _type    << std::endl;
    std::cout << "_content: " << _content << std::endl;
}

Glib::ustring XmlMeasurement::to_xml() const
{
    return _content;
}
