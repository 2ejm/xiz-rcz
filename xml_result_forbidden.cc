#include "xml_result_forbidden.h"

XmlResultForbidden::XmlResultForbidden ()
    : XmlResult (403)
{ }

XmlResultForbidden::XmlResultForbidden (const Glib::ustring & message)
    : XmlResult (403, message)
{ }

Glib::RefPtr <XmlResult>
XmlResultForbidden::create ()
{
    auto res = Glib::RefPtr <XmlResultForbidden> (new XmlResultForbidden ());

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}

Glib::RefPtr <XmlResult>
XmlResultForbidden::create (const Glib::ustring & message)
{
    auto res = Glib::RefPtr <XmlResultForbidden> (new XmlResultForbidden (message));

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}
