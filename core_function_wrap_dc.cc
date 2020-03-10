
#include "core_function_wrap_dc.h"

#include "query_client.h"
#include "xml_query_pass.h"
#include "xml_helpers.h"
#include "procedure_step_handler.h"
#include "utils.h"
#include <cassert>

CoreFunctionWrapDC::CoreFunctionWrapDC (const Glib::ustring & fid,
					XmlParameterList parameters,
					Glib::RefPtr<XmlDescription> description,
					const Glib::ustring & textbody,
					const xmlpp::Element * en,
					bool use_timeout)
    : CoreFunctionCall (fid, parameters, description, textbody)
    , _use_timeout (use_timeout)
{
    lDebug ("CoreFunctionWrapDC::CoreFunctionWrapDC fid=%s\n", fid.c_str());
    /* we are supposed to pass the xml through
     *
     * however, we need to remove a few fields.
     *
     * instantiate a document with a copy of the
     * <function> element.
     */
    xmlpp::Document doc;
    xmlpp::Node *n;
    doc.create_root_node_by_import(en, true);

    xmlpp::Node *root = doc.get_root_node();

    /* remove possibly existing <signature>
     */
    while ((n = root->get_first_child("signature"))) {
	root->remove_child(n);
    }

    /* remove possibly existing <restriction>
     */
    while ((n = root->get_first_child("restriction"))) {
	root->remove_child(n);
    }

    /* remove possibly existing <description> if executed in procedure context
     */
    if (procedure_in_progress()) {
        while ((n = root->get_first_child("description"))) {
            root->remove_child(n);
        }
    }

    _xml = node_to_string (root, 0, 0);
}

CoreFunctionWrapDC::~CoreFunctionWrapDC ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_get_measurement_list (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("getMeasurementList", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_get_calibration (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("getCalibration", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_get_defaults (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("getDefaults", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_get_profiles_list (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("getProfilesList", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_get_profile (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("getProfile", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_get_parameters_list (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("getParametersList", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_get_parameter (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
        new CoreFunctionWrapDC ("getParameter", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_get_templates_list (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("getTemplatesList", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_get_template (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("getTemplate", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_set_measurements_list (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("setMeasurementsList", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_set_measurement (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("setMeasurement", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_set_calibration (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("setCalibration", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_set_defaults (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("setDefaults", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_set_profiles_list (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("setProfilesList", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_set_profile (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("setProfile", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_set_parameter (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("setParameter", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_set_templates_list (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("setTemplatesList", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_set_template (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("setTemplate", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_del_measurement (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("delMeasurement", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_del_profile (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("delProfile", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_del_template (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("delTemplate", parameters, description, textbody, en));
}

Glib::RefPtr <CoreFunctionCall>
CoreFunctionWrapDC::factory_gui_io (
	XmlParameterList parameters,
	Glib::RefPtr <XmlDescription> description,
	const Glib::ustring & textbody,
	const xmlpp::Element * en)
{
    /* guiIO is the only call,
     * the has use_timeout = false set
     */
    return Glib::RefPtr<CoreFunctionCall> (
	    new CoreFunctionWrapDC ("guiIO", parameters, description, textbody, en, false));
}

void
CoreFunctionWrapDC::on_query_finish (const Glib::RefPtr<XmlResult> result)
{
    lDebug ("CoreFunctionWrapDC::on_query_finish fid=%s result_status=%d\n", _fid.c_str(), result->get_status ());
    call_finished (result);
}

void
CoreFunctionWrapDC::start_call ()
{
    lDebug ("CoreFunctionWrapDC::start_call\n");

    auto dc_client = QueryClient::get_instance ();

    auto xml_query = XmlQueryPass::create ( _fid, _xml );

    _dc_query = dc_client->create_query (xml_query, _use_timeout);
    _dc_query->finished.connect(
	    sigc::mem_fun(*this, &CoreFunctionWrapDC::on_query_finish));
    dc_client->execute(_dc_query);
}

