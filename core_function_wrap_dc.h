
#ifndef ZIX_CORE_FUNCTION_WRAP_DC
#define ZIX_CORE_FUNCTION_WRAP_DC

#include <libxml++/parsers/domparser.h>

#include "core_function_call.h"
#include "query_client.h"

class CoreFunctionWrapDC : public CoreFunctionCall
{
    public:
	CoreFunctionWrapDC (const Glib::ustring & fid,
			    XmlParameterList parameters,
			    Glib::RefPtr<XmlDescription> description,
			    const Glib::ustring & textbody,
			    const xmlpp::Element * en,
			    bool use_timeout = true);

	~CoreFunctionWrapDC();

	void start_call();

	static Glib::RefPtr <CoreFunctionCall> factory_get_measurement_list (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_get_calibration (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_get_defaults (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_get_profiles_list (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_get_profile (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_get_parameters_list (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_get_parameter (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_get_templates_list (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_get_template (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_set_measurements_list (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_set_measurement (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_set_calibration (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_set_defaults (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_set_profiles_list (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_set_profile (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_set_parameter (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_set_templates_list (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_set_template (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_del_measurement (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_del_profile (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_del_template (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

	static Glib::RefPtr <CoreFunctionCall> factory_gui_io (
		XmlParameterList parameters,
		Glib::RefPtr <XmlDescription> description,
		const Glib::ustring & textbody,
		const xmlpp::Element * en);

    private:
        Glib::RefPtr <Query> _dc_query;
	bool _use_timeout;
        void on_query_finish (const Glib::RefPtr<XmlResult> result);
        Glib::ustring _xml;

};


#endif
