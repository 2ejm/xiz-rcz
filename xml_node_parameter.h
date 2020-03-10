
#ifndef ZIX_XML_NODE_PARAMETER
#define ZIX_XML_NODE_PARAMETER

#include "xml_parameter.h"

#include <libxml++/nodes/element.h>
/**
 * \brief Special XmlParameter that maps a whole xmlpp::Entity
 *
 * Generic XmlParameter for "unknown" Parameter Types, that we just pass through
 * to DC.
 * We dont register to the XmlParameter Factory, because this is the Fallback.
 */
class XmlNodeParameter : public XmlParameter
{
    public:
	XmlNodeParameter (const xmlpp::Element * en);

	static Glib::RefPtr <XmlNodeParameter> create (const xmlpp::Element * en);

	Glib::ustring to_xml () const;

	void dump ();

    private:
	Glib::ustring _xml;
};
#endif
