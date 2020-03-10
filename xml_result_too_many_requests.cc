#include "xml_result_too_many_requests.h"

XmlResultTooManyRequests::XmlResultTooManyRequests ()
    : XmlResult (429)
{ }

XmlResultTooManyRequests::XmlResultTooManyRequests (const Glib::ustring & message)
    : XmlResult (429, message)
{ }

Glib::RefPtr <XmlResult>
XmlResultTooManyRequests::create ()
{
    auto res = Glib::RefPtr <XmlResultTooManyRequests> (new XmlResultTooManyRequests ());

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}

Glib::RefPtr <XmlResult>
XmlResultTooManyRequests::create (const Glib::ustring & message)
{
    auto res = Glib::RefPtr <XmlResultTooManyRequests> (new XmlResultTooManyRequests (message));

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}
