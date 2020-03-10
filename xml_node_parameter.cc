

#include "xml_node_parameter.h"
#include "xml_helpers.h"

#include <libxml++/document.h>

XmlNodeParameter::XmlNodeParameter (const xmlpp::Element *en)
    : XmlParameter ("node", en->get_name ())
{
    _xml = element_to_string(en, 0, 0);
}

Glib::RefPtr <XmlNodeParameter>
XmlNodeParameter::create (const xmlpp::Element *en)
{
    return Glib::RefPtr <XmlNodeParameter> (new XmlNodeParameter (en));
}

Glib::ustring
XmlNodeParameter::to_xml () const
{
    return _xml;
}

void
XmlNodeParameter::dump ()
{
    printf ("node param: name=%s\n", get_name().c_str());
    printf ("xml ---------------------------------\n");
    printf ("%s\n", _xml.c_str());
    printf ("end ---------------------------------\n");
}
