#include "xml_query_set_parameter.h"

Glib::ustring XmlQuerySetParameter::to_xml() const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose("<function fid=\"%1\">\n", _fid);
    ret += Glib::ustring::compose("<parameter id=\"%1\" value=\"%2\"/>\n", _id, _value);
    ret += "</function>\n";

    return ret;
}
