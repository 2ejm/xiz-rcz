#ifndef _XML_RESULT_NOT_FOUND_H_
#define _XML_RESULT_NOT_FOUND_H_

#include "xml_result.h"

/**
 * \brief NotFound XmlResult
 *
 * Used for File API.
 */
class XmlResultNotFound : public XmlResult
{
    public:
	XmlResultNotFound ();
	XmlResultNotFound (const Glib::ustring & message);

	static Glib::RefPtr <XmlResult> create ();
	static Glib::RefPtr <XmlResult> create (const Glib::ustring & message);
};

#endif /* _XML_RESULT_NOT_FOUND_H_ */
