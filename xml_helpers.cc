
#include "xml_helpers.h"

#include "utils.h"

#include <libxml++/nodes/element.h>
#include <libxml++/nodes/contentnode.h>
#include <libxml++/nodes/node.h>
#include <libxml++/libxml++.h>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>

Glib::ustring
xml_element_pure_text (const xmlpp::Element *en)
{
    auto l = en->get_children ();

    if (l.size () != 1)
	return Glib::ustring ();

    const xmlpp::ContentNode *cn = dynamic_cast <const xmlpp::ContentNode *> ( l.front());
    if (cn == nullptr)
	return Glib::ustring ();

    return cn->get_content ();
}

Glib::ustring
doc_to_string (xmlpp::Document *doc, int format)
{
    int save_flag = XML_SAVE_NO_DECL;
    Glib::ustring ret;
    int err;

    xmlBufferPtr buffer = xmlBufferCreate();

    if (format)
	save_flag |= XML_SAVE_WSNONSIG;

    xmlSaveCtxtPtr save_ctx = xmlSaveToBuffer (buffer, "UTF-8", save_flag);
    if (save_ctx == 0) {
	PRINT_ERROR ("doc_to_string: xmlSaveToBuffer returns error");
	xmlBufferFree (buffer);
    }

    err = xmlSaveDoc (save_ctx, doc->cobj ());
    if (err == -1) {
	PRINT_ERROR ("doc_to_string: xmlSaveDoc returns error");
	xmlSaveClose (save_ctx);
	xmlBufferFree (buffer);
	return ret;
    }

    err = xmlSaveClose (save_ctx);
    if (err == -1) {
	PRINT_ERROR ("doc_to_string: xmlSaveClose returns error");
	xmlBufferFree (buffer);
	return ret;
    }

    if (buffer->use) {
	ret=Glib::ustring (reinterpret_cast <const char *> (buffer->content),
		           reinterpret_cast <const char *> (buffer->content + buffer->use));
    }

    xmlBufferFree (buffer);
    return ret;
}

Glib::ustring
element_to_string_no_recurse (const xmlpp::Element *element)
{
    xmlpp::Document doc;

    doc.create_root_node_by_import (element);

    xmlpp::Element * rn = doc.get_root_node ();

    if (rn == 0) {
	PRINT_ERROR ("element_to_string_no_recurse: failed to access imported root node");
	return Glib::ustring ();
    }

    for (xmlpp::Node *c : rn->get_children ()) {
	rn->remove_child (c);
    }

    return doc_to_string (&doc, 0);
}

Glib::ustring
element_to_string_recurse_one (const xmlpp::Element *element)
{
    xmlpp::Document doc;

    doc.create_root_node_by_import (element);

    xmlpp::Element * rn = doc.get_root_node ();

    if (rn == 0) {
	PRINT_ERROR ("element_to_string_no_recurse: failed to access imported root node");
	return Glib::ustring ();
    }

    for (xmlpp::Node *c : rn->get_children ()) {
        for (auto *sub : c->get_children())
            rn->remove_child (sub);
    }

    return doc_to_string (&doc, 0);
}

Glib::ustring
element_to_string (const xmlpp::Element *element, int indent, int format)
{
    (void) indent;

    xmlpp::Document doc;

    doc.create_root_node_by_import (element);

    return doc_to_string (&doc, format);
}

Glib::ustring
node_to_string (const xmlpp::Node *node, int indent, int format)
{
    (void) indent;

    xmlpp::Document doc;

    doc.create_root_node_by_import (node);

    return doc_to_string (&doc, format);
}

Glib::ustring
xml_escape (const Glib::ustring & raw)
{
    Glib::ustring ret;

    for (auto ch : raw) {
	switch (ch) {
	    case '&':  ret.append ("&amp;"); break;
	    case '\'': ret.append ("&apos;"); break;
	    case '"':  ret.append ("&quot;"); break;
	    case '<':  ret.append ("&lt;"); break;
	    case '>':  ret.append ("&gt;"); break;
	    default:   ret.push_back (ch); break;
	}
    }
    return ret;
}
