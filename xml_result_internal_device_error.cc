#include "xml_result_internal_device_error.h"

XmlResultInternalDeviceError::XmlResultInternalDeviceError ()
    : XmlResult (500)
{ }

XmlResultInternalDeviceError::XmlResultInternalDeviceError (const Glib::ustring & message)
    : XmlResult (500, message)
{ }

XmlResultInternalDeviceError::XmlResultInternalDeviceError (const Glib::ustring & message,
							    const std::vector<Glib::ustring>& return_values)
    : XmlResult (500, message, return_values)
{ }

Glib::RefPtr <XmlResult>
XmlResultInternalDeviceError::create ()
{
    auto res = Glib::RefPtr <XmlResultInternalDeviceError> (new XmlResultInternalDeviceError ());

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}

Glib::RefPtr <XmlResult>
XmlResultInternalDeviceError::create (const Glib::ustring & message)
{
    auto res = Glib::RefPtr <XmlResultInternalDeviceError> (new XmlResultInternalDeviceError (message));

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}

Glib::RefPtr <XmlResult>
XmlResultInternalDeviceError::create (const Glib::ustring & message,
				      const std::vector<Glib::ustring>& return_values)
{
    auto res = Glib::RefPtr <XmlResultInternalDeviceError> (new XmlResultInternalDeviceError (message, return_values));

    return Glib::RefPtr <XmlResult>::cast_dynamic (res);
}
