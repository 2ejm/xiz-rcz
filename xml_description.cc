
#include "xml_description.h"

#include <cassert>

#include "xml_helpers.h"

XmlDescription::XmlDescription (const xmlpp::Element *en)
    : XmlParameter ("description", "description")
{
    assert (en->get_name() == "description");

    /* check whether we have
     * <description>text</description>
     */
    Glib::ustring val = xml_element_pure_text (en);
    if (! val.empty()) {
	    _description ["default"] = val;
	    _default_lang = "default";
	    return;
    }

    /* fallthrough, iterate over the description tags
     */

    for (auto c : en->get_children ()) {
	const xmlpp::Element *en = dynamic_cast<const xmlpp::Element *> (c);

	if (en == nullptr)
	    /* not an Element, ignore
	     */
	    continue;

	/* looking at an Element
	 */

	assert (en->get_name() == "language");

	const Glib::ustring &lang_name    = en->get_attribute_value ("name");
	const Glib::ustring &lang_default = en->get_attribute_value ("default");
	const Glib::ustring &lang_text = xml_element_pure_text (en);

	_description[lang_name] = lang_text;

	if (! lang_default.empty())
	    _default_lang = lang_name;
    }
}

XmlDescription::~XmlDescription ()
{ }

Glib::RefPtr <XmlDescription>
XmlDescription::create (const xmlpp::Element *en)
{
    return Glib::RefPtr <XmlDescription> (new XmlDescription (en));
}

Glib::RefPtr <XmlParameter>
XmlDescription::param_factory (const xmlpp::Element *en)
{
    return Glib::RefPtr <XmlParameter>::cast_dynamic (create (en));
}

const Glib::ustring &
XmlDescription::get_description (const Glib::ustring & lang)
{
    return _description [lang];
}


void
XmlDescription::dump ()
{
    printf ("_description:\n");
    for (auto d : _description) {
	printf (" %s = %s\n", d.first.c_str(), d.second.c_str());
    }
}

Glib::ustring
XmlDescription::to_body () const
{
    Glib::ustring ret;

    if (_default_lang == "default")
	return _description.at("default");

    for (auto l : _description) {
	auto lang = l.first;
	auto text = l.second;

	if (lang == _default_lang)
	    ret += Glib::ustring::compose ("<language name=\"%1\" default=\"1\">%2</language>\n", lang, text);
	else
	    ret += Glib::ustring::compose ("<language name=\"%1\">%2</language>\n", lang, text);
    }

    return ret;
}
Glib::ustring
XmlDescription::to_xml () const
{
    Glib::ustring ret;

    ret += "<description>\n";
    ret += to_body ();
    ret += "</description>\n";

    return ret;
}
