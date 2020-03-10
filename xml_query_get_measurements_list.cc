#include "xml_query_get_measurements_list.h"

XmlQueryGetMeasurementsList::XmlQueryGetMeasurementsList(const Glib::ustring& scope)
    : XmlQuery ("getMeasurementsList")
    , _scope (scope)
{ }

Glib::RefPtr<XmlQuery>
XmlQueryGetMeasurementsList::create(const Glib::ustring& scope)
{
    return Glib::RefPtr<XmlQuery>(new XmlQueryGetMeasurementsList (scope));
}

XmlQueryGetMeasurementsList::~XmlQueryGetMeasurementsList()
{ }

Glib::ustring
XmlQueryGetMeasurementsList::to_xml() const
{
    return Glib::ustring::compose("<function fid=\"%1\" scope=\"%2\" />\n", _fid, _scope);
}
