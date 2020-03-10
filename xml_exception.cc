
#include "xml_exception.h"

XmlException::XmlException (const Glib::ustring & msg)
    : _msg (msg)
{ }

const char *
XmlException::what () const noexcept
{
    return _msg.c_str();
}

XmlExceptionUnknownFid::XmlExceptionUnknownFid (const Glib::ustring & fid)
    : XmlException (Glib::ustring::compose ("Unknown Fid %1 found in xml", fid))
{ }

XmlExceptionInvalidParameter::XmlExceptionInvalidParameter (const Glib::ustring & name)
    : XmlException (Glib::ustring::compose ("Unknown Parameter Construct %1 found in xml", name))
{ }

XmlExceptionNoAccess::XmlExceptionNoAccess (const Glib::ustring & fid, const Glib::ustring & interface)
    : XmlException (Glib::ustring::compose ("Function '%1' is not available via interface '%2'", fid, interface))
{ }

