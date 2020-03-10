
#ifndef ZIX_XML_RESULT_PARSED
#define ZIX_XML_RESULT_PARSED

#include "xml_result.h"
#include "libxml++/parsers/domparser.h"

namespace xmlpp
{
    class DomParser;
}

class XmlResultParsed : public XmlResult
{
    public:
	XmlResultParsed (const Glib::ustring & xml);
	XmlResultParsed (const unsigned char* contents, int bytes_count);
	XmlResultParsed (const xmlpp::Element *root);

	static Glib::RefPtr <XmlResult> create (const Glib::ustring & xml);
	static Glib::RefPtr <XmlResult> create (const unsigned char* contents, int bytes_count);
	static Glib::RefPtr <XmlResult> create (const xmlpp::Element *root);

	void parse (const xmlpp::Element *root);

    private:
	xmlpp::DomParser _parser;
};

#endif
