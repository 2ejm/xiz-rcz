#include <cassert>
#include <iostream>

#include "xml_helpers.h"

#include "xml_restriction.h"

XmlRestriction::XmlRestriction(const xmlpp::Element *en) :
    XmlParameter("restriction", "restriction")
{
    assert(en->get_name() == "restriction");

    for (auto&& c : en->get_children()) {
        const xmlpp::Element *elem = dynamic_cast<const xmlpp::Element *>(c);

        if (!elem)
            continue;

        assert(elem->get_name() == "parameter");

        _params.push_back(XmlRestrictionParameter::create(elem));
    }
}

XmlRestriction::~XmlRestriction ()
{ }

Glib::RefPtr<XmlRestriction>
XmlRestriction::create(const xmlpp::Element *en)
{
    return Glib::RefPtr<XmlRestriction>(new XmlRestriction(en));
}

Glib::RefPtr <XmlParameter>
XmlRestriction::param_factory(const xmlpp::Element *en)
{
    return Glib::RefPtr<XmlParameter>::cast_dynamic(create(en));
}

void XmlRestriction::dump()
{
    std::cout << "_params:" << std::endl;

    for (auto&& p : _params)
        p->dump();
}

Glib::ustring XmlRestriction::to_xml() const
{
    Glib::ustring ret;

    ret = "<restriction>\n";
    for (auto&& p : _params)
        ret += "  " + p->to_xml();
    ret += "</restriction>\n";

    return ret;
}
