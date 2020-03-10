
#ifndef ZIX_XML_RESULT_OK_H
#define ZIX_XML_RESULT_OK_H

#include "xml_result.h"

/**
 * \brief Good Result of CreFunctionCall
 */
class XmlResultOk : public XmlResult
{
    public:
	XmlResultOk ();
	XmlResultOk (const Glib::ustring & message);
    XmlResultOk (const Glib::ustring & message,
                 const std::vector<Glib::ustring>& return_values);

	static Glib::RefPtr <XmlResult> create ();
	static Glib::RefPtr <XmlResult> create (const Glib::ustring & message);
    static Glib::RefPtr <XmlResult> create (const Glib::ustring & message,
                                            const std::vector<Glib::ustring>& return_values);
};
#endif
