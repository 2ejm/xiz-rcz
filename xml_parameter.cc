
#include "xml_parameter.h"

#include "xml_helpers.h"
#include "xml_string_parameter.h"
#include "xml_node_parameter.h"
#include "xml_exception.h"

#include "xml_function.h"
#include "xml_description.h"
#include "xml_resource.h"
#include "xml_file.h"
#include "xml_measurement.h"
#include "xml_image.h"
#include "xml_conf_parameter.h"

XmlParameter::XmlParameter (const Glib::ustring & type, const Glib::ustring & name)
    : _type (type)
    , _name (name)
{ }

Glib::RefPtr <XmlParameter>
XmlParameter::create (const xmlpp::Element *en)
{
    const Glib::ustring & name = en->get_name ();
    Glib::ustring val;

    /* try to create XmlParameter via factory map
     */
    auto f_iter = factory_map.find (name);
    if (f_iter != factory_map.end ()) {
	return f_iter->second (en);
    }

    /* fallthrough
     * parameter type not found in factory_map
     *
     * check whether we have
     * <arg>text</arg>
     */
    val = xml_element_pure_text (en);
    if (! val.empty()) {
	    return XmlStringParameter::create (name, val);
    }

    // TODO: make sure that en doesnt have children, when the value pattern matches.
    val = en->get_attribute_value ("value");
    if (! val.empty()) {
	/* this is <arg value="x"/>
	 */
	return XmlStringParameter::create (name, val);
    }

    return XmlNodeParameter::create (en);
}

std::map <Glib::ustring, XmlParameter::Factory>
XmlParameter::factory_map;

void
XmlParameter::register_factory (const Glib::ustring & name, XmlParameter::Factory factory)
{
    factory_map [name] = factory;
}

void
XmlParameter::init ()
{
    register_factory ("description", & XmlDescription::param_factory);
    register_factory ("resource", & XmlResource::param_factory);
    register_factory ("file", & XmlFile::param_factory);
    register_factory ("measurement", & XmlMeasurement::param_factory);
    register_factory ("image", & XmlImage::param_factory);
    register_factory ("parameter", & XmlConfParameter::param_factory);
}
