
#ifndef ZIX_XML_RESULT_BAD_REQUEST_H
#define ZIX_XML_RESULT_BAD_REQUEST_H

#include "xml_result.h"

/**
 * \brief BadRequest XmlResult
 *
 * Exception was thrown during XmlProcessor processing
 */
class XmlResultBadRequest : public XmlResult
{
    public:
	XmlResultBadRequest ();
	XmlResultBadRequest (const Glib::ustring & message);

	static Glib::RefPtr <XmlResult> create ();
	static Glib::RefPtr <XmlResult> create (const Glib::ustring & message);
};
#endif
