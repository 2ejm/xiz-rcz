
#ifndef ZIX_XML_HELPERS_H
#define ZIX_XML_HELPERS_H

#include <glibmm/ustring.h>

#include <libxml++/nodes/element.h>

/**
 * \brief check whether xmlpp::Element only contains Text
 *
 * This distinguishes String Parameters from Structured Parameters
 *
 * \returns Empty Glib::ustring, when not a pure Text element.
 */
Glib::ustring xml_element_pure_text (const xmlpp::Element *node);

Glib::ustring
element_to_string_no_recurse (const xmlpp::Element *element);

Glib::ustring
element_to_string_recurse_one (const xmlpp::Element *element);

Glib::ustring
element_to_string (const xmlpp::Element *element, int indent, int format);

Glib::ustring
node_to_string (const xmlpp::Node *element, int indent, int format);

Glib::ustring xml_escape (const Glib::ustring & raw);
#endif
