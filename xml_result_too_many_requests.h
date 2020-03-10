#ifndef _XML_RESULT_TOO_MANY_REQUESTS_H_
#define _XML_RESULT_TOO_MANY_REQUESTS_H_

#include "xml_result.h"

/**
 * \brief Too Many Requests XmlResult
 */
class XmlResultTooManyRequests : public XmlResult
{
public:
    XmlResultTooManyRequests ();
    XmlResultTooManyRequests (const Glib::ustring & message);

    static Glib::RefPtr <XmlResult> create ();
    static Glib::RefPtr <XmlResult> create (const Glib::ustring & message);
};

#endif /* _XML_RESULT_TOO_MANY_REQUESTS_H_ */
