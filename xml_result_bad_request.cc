
#include "xml_result_bad_request.h"

XmlResultBadRequest::XmlResultBadRequest ()
    : XmlResult (400)
{ }

XmlResultBadRequest::XmlResultBadRequest (const Glib::ustring & message)
    : XmlResult (400, message)
{ }

Glib::RefPtr <XmlResult>
XmlResultBadRequest::create ()
{
    return Glib::RefPtr <XmlResult> (new XmlResultBadRequest ());
}

Glib::RefPtr <XmlResult>
XmlResultBadRequest::create (const Glib::ustring & message)
{
    return Glib::RefPtr <XmlResult> (new XmlResultBadRequest (message));
}
