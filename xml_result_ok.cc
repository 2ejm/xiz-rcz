
#include "xml_result_ok.h"

XmlResultOk::XmlResultOk ()
    : XmlResult (200)
{ }

XmlResultOk::XmlResultOk (const Glib::ustring & message)
    : XmlResult (200, message)
{ }

XmlResultOk::XmlResultOk (const Glib::ustring & message,
                          const std::vector<Glib::ustring>& return_values)
    : XmlResult (200, message, return_values)
{ }

Glib::RefPtr <XmlResult>
XmlResultOk::create ()
{
    auto res = Glib::RefPtr <XmlResultOk> (new XmlResultOk ());

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}

Glib::RefPtr <XmlResult>
XmlResultOk::create (const Glib::ustring & message)
{
    auto res = Glib::RefPtr <XmlResultOk> (new XmlResultOk (message));

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}

Glib::RefPtr <XmlResult>
XmlResultOk::create (const Glib::ustring & message,
                     const std::vector<Glib::ustring>& return_values)
{
    auto res = Glib::RefPtr <XmlResultOk> (new XmlResultOk (message, return_values));

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}
