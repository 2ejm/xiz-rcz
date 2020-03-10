#include "xml_query_set_datetime.h"

Glib::ustring XmlQuerySetDateTime::to_xml() const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose("<function fid=\"%1\">\n", _fid);
    ret += Glib::ustring::compose("<parameter id=\"date\" value=\"%1\"/>\n", _date);
    ret += Glib::ustring::compose("<parameter id=\"time\" value=\"%1\"/>\n", _time);
    ret += "</function>\n";

    return ret;
}
