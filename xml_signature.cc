#include <cassert>
#include <iostream>

#include "xml_helpers.h"

#include "xml_signature.h"

XmlSignature::XmlSignature(const xmlpp::Element *en) :
    XmlParameter("signature", "signature")
{
    assert(en->get_name() == "signature");

    _signature = xml_element_pure_text(en);
}

XmlSignature::~XmlSignature ()
{ }

Glib::RefPtr<XmlSignature>
XmlSignature::create(const xmlpp::Element *en)
{
    return Glib::RefPtr<XmlSignature>(new XmlSignature(en));
}

Glib::RefPtr <XmlParameter>
XmlSignature::param_factory(const xmlpp::Element *en)
{
    return Glib::RefPtr<XmlParameter>::cast_dynamic(create(en));
}

void XmlSignature::dump()
{
    std::cout << "_signature:" << _signature << std::endl;
}

Glib::ustring XmlSignature::to_xml() const
{
    Glib::ustring ret;

    ret  = "<signature>\n";
    ret += _signature + "\n";
    ret += "</signature>\n";

    return ret;
}
