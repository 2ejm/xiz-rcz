#include <iostream>
#include <cassert>
#include <libxml++/parsers/domparser.h>

#include "xml_image.h"
#include "xml_helpers.h"
#include "xml_exception.h"


XmlImage::XmlImage (const xmlpp::Element *en)
    : XmlParameter("image", "image")
{
    assert(en);
    assert(en->get_name() == "image");

    _type = en->get_attribute_value("type");

    for (auto c : en->get_children ()) {
	const xmlpp::Element *cen = dynamic_cast<const xmlpp::Element *> (c);

	if (cen == nullptr)
	    /* not an Element, ignore
	     */
	    continue;

	/* looking at an Element
	 */

	if (cen->get_name() == "signature") {
	    /* signature element contenst is saved to _signature
	     */
	    _signature = xml_element_pure_text (cen);
	} else if (cen->get_name() == "content") {
	    /* content node
	     * this is path to a file, or uuencoded binary data
	     * depending on type
	     */
	    _content = xml_element_pure_text (cen);
	} else {
	    /* unknown element in image node
	     * lets ignore it for now
	     */
	    continue;
	}
    }

    if (_signature.empty())
	throw XmlException ("image tag must contain (text-only) signature tag.");

    if (_content.empty())
	throw XmlException ("image tag must contain (text-only) content tag.");
}


XmlImage::~XmlImage ()
{ }

Glib::RefPtr <XmlImage>
XmlImage::create(const xmlpp::Element *en)
{
    return Glib::RefPtr<XmlImage>(new XmlImage(en));
}


Glib::RefPtr<XmlParameter>
XmlImage::param_factory(const xmlpp::Element *en)
{
    return Glib::RefPtr<XmlParameter>::cast_dynamic(create(en));
}

void XmlImage::dump()
{
    std::cout << "_type: "    << _type    << std::endl;
    std::cout << "_content: " << _content << std::endl;
    std::cout << "_signature: " << _signature << std::endl;
}

Glib::ustring XmlImage::to_xml() const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose("<image type=\"%1\">\n", _type);
    ret += "<signature>\n";
    ret += _signature;
    ret += "</signature>\n";
    ret += "<content>\n";
    ret += _content;
    ret += "</content>\n";

    return ret;
}
