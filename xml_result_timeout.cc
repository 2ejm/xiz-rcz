#include "xml_result_timeout.h"

XmlResultTimeout::XmlResultTimeout ()
    : XmlResult (408)
{ }

XmlResultTimeout::XmlResultTimeout (const Glib::ustring & message)
    : XmlResult (408, message)
{ }

Glib::RefPtr <XmlResult>
XmlResultTimeout::create ()
{
    auto res = Glib::RefPtr <XmlResultTimeout> (new XmlResultTimeout ());

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}

Glib::RefPtr <XmlResult>
XmlResultTimeout::create (const Glib::ustring & message)
{
    auto res = Glib::RefPtr <XmlResultTimeout> (new XmlResultTimeout (message));

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}
