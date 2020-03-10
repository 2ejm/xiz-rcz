#include "xml_result_not_found.h"

XmlResultNotFound::XmlResultNotFound ()
    : XmlResult (404)
{ }

XmlResultNotFound::XmlResultNotFound (const Glib::ustring & message)
    : XmlResult (404, message)
{ }

Glib::RefPtr <XmlResult>
XmlResultNotFound::create ()
{
    auto res = Glib::RefPtr <XmlResultNotFound> (new XmlResultNotFound ());

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}

Glib::RefPtr <XmlResult>
XmlResultNotFound::create (const Glib::ustring & message)
{
    auto res = Glib::RefPtr <XmlResultNotFound> (new XmlResultNotFound (message));

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}
