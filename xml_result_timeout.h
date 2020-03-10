#ifndef _XML_RESULT_TIMEOUT_H_
#define _XML_RESULT_TIMEOUT_H_

#include "xml_result.h"

/**
 * \brief Timeout XmlResult
 *
 * Used for File API.
 */
class XmlResultTimeout: public XmlResult
{
    public:
    XmlResultTimeout ();
    XmlResultTimeout (const Glib::ustring & message);

	static Glib::RefPtr <XmlResult> create ();
	static Glib::RefPtr <XmlResult> create (const Glib::ustring & message);
};

#endif /* _XML_RESULT_TIMEOUT_H_ */
