#ifndef _XML_RESULT_FORBIDDEN_H_
#define _XML_RESULT_FORBIDDEN_H_

#include "xml_result.h"

/**
 * \brief Forbidden XmlResult
 *
 * Used for File API.
 */
class XmlResultForbidden : public XmlResult
{
    public:
	XmlResultForbidden ();
	XmlResultForbidden (const Glib::ustring & message);

	static Glib::RefPtr <XmlResult> create ();
	static Glib::RefPtr <XmlResult> create (const Glib::ustring & message);
};

#endif /* _XML_RESULT_FORBIDDEN_H_ */
