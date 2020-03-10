#ifndef _XML_RESULT_INTERNAL_DEVICE_ERROR_H_
#define _XML_RESULT_INTERNAL_DEVICE_ERROR_H_

#include "xml_result.h"

/**
 * \brief Internal Device Error XmlResult
 */
class XmlResultInternalDeviceError : public XmlResult
{
public:
    XmlResultInternalDeviceError ();
    XmlResultInternalDeviceError (const Glib::ustring & message);
    XmlResultInternalDeviceError (const Glib::ustring & message,
				  const std::vector<Glib::ustring>& return_values);

    static Glib::RefPtr <XmlResult> create ();
    static Glib::RefPtr <XmlResult> create (const Glib::ustring & message);
    static Glib::RefPtr <XmlResult> create (const Glib::ustring & message,
                                            const std::vector<Glib::ustring>& return_values);
};

#endif /* _XML_RESULT_INTERNAL_DEVICE_ERROR_H_ */
