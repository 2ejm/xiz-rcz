#include <cassert>
#include <cstdint>
#include <iostream>
#include <regex>
#include <sstream>

#include "xml_helpers.h"
#include "utils.h"

#include "xml_restriction_parameter.h"

XmlRestrictionParameter::XmlRestrictionParameter(const xmlpp::Element *en) :
    XmlParameter("parameter", "parameter")
{
    assert(en->get_name() == "parameter");

    _id    = en->get_attribute_value("id");
    _exp   = en->get_attribute_value("exp");
    _nexp  = en->get_attribute_value("nexp");
    _value = en->get_attribute_value("value");
}

XmlRestrictionParameter::XmlRestrictionParameter (const Glib::ustring & id)
    : XmlParameter ("parameter", "parameter")
    , _id (id)
{ }

XmlRestrictionParameter::~XmlRestrictionParameter ()
{ }

Glib::RefPtr<XmlRestrictionParameter>
XmlRestrictionParameter::create(const xmlpp::Element *en)
{
    return Glib::RefPtr<XmlRestrictionParameter>(new XmlRestrictionParameter(en));
}

Glib::RefPtr<XmlRestrictionParameter>
XmlRestrictionParameter::create (const Glib::ustring & id)
{
    return Glib::RefPtr <XmlRestrictionParameter> (new XmlRestrictionParameter (id));
}

Glib::RefPtr <XmlParameter>
XmlRestrictionParameter::param_factory(const xmlpp::Element *en)
{
    return Glib::RefPtr<XmlParameter>::cast_dynamic(create(en));
}

void XmlRestrictionParameter::dump()
{
    std::cout << "_id: "    << _id << std::endl;
    std::cout << "_exp: "   << _exp << std::endl;
    std::cout << "_nexp: "  << _nexp << std::endl;
    std::cout << "_value: " << _value << std::endl;
}

Glib::ustring XmlRestrictionParameter::to_xml() const
{
    Glib::ustring ret;

    ret = "<parameter ";
    if (!_id.empty())
        ret += Glib::ustring::compose("id=\"%1\" ", _id);
    if (!_exp.empty())
        ret += Glib::ustring::compose("exp=\"%1\" ", _exp);
    if (!_nexp.empty())
        ret += Glib::ustring::compose("nexp=\"%1\" ", _nexp);
    if (!_value.empty())
        ret += Glib::ustring::compose("value=\"%1\" ", _value);
    ret += "/>\n";

    return ret;
}

bool XmlRestrictionParameter::evaluate() const
{
    const Glib::ustring& exp = _exp.empty() ? _nexp : _exp;
    std::string val, op;
    std::uint64_t num1, num2;
    bool result = true;

    // sanity checks come first
    if (_value.empty())
        EXCEPTION("Cannot evaluate restriction parameter b/o no value given");
    if (!_exp.empty() && !_nexp.empty())
        EXCEPTION("Exp and Nexp given for a single restriction parameter");

    // parse exp
    std::regex re("([=<>]{1,2})([0-9]+)");
    std::smatch match;

    if (!std::regex_match(exp.raw(), match, re))
        EXCEPTION("Expression " << exp << " in restriction parameter isn't valid");

    op  = match[1];
    val = match[2];

    // convert string to numbers, seems like we can use unsigned
    std::istringstream(_value) >> num1;
    std::istringstream(val)    >> num2;

    // switch op
    if (op == "=")
        result = num1 == num2;
    else if (op == "<")
        result = num1 < num2;
    else if (op == "<=")
        result = num1 <= num2;
    else if (op == ">")
        result = num1 > num2;
    else if (op == ">=")
        result = num1 >= num2;
    else
        EXCEPTION("Operator " << op << " is invalid for restriction parameter");

    return _nexp.empty() ? result : !result;
}
