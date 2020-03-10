#include <stdexcept>

#include "xml_string_parameter.h"
#include "xml_query_get_measurements_list.h"
#include "conf_handler.h"
#include "id_mapper.h"
#include "utils.h"

#include "core_function_get_measurements_list.h"

CoreFunctionGetMeasurementsList::CoreFunctionGetMeasurementsList(
    XmlParameterList parameters,
    Glib::RefPtr <XmlDescription> description,
    const Glib::ustring& text)
    : CoreFunctionCall ("getMeasurementsList", parameters, description, text)
{ }

CoreFunctionGetMeasurementsList::~CoreFunctionGetMeasurementsList ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionGetMeasurementsList::factory(XmlParameterList parameters,
                              Glib::RefPtr <XmlDescription> description,
                              const Glib::ustring & text,
			      const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr<CoreFunctionCall>(
        new CoreFunctionGetMeasurementsList(parameters, description, text));
}

bool CoreFunctionGetMeasurementsList::check_xml_and_get_values()
{
    const auto& target_param = _parameters.get<XmlStringParameter>("target");
    const auto& scope_param  = _parameters.get<XmlStringParameter>("scope");

    _scope  = scope_param  ? scope_param->get_str()  : "";
    _target = target_param ? target_param->get_str() : "";

    if (_target.empty())
        _target = "DC";

    if (_target != "DC" && _target != "SIC")
        return false;

    if (_scope.empty() && _target == "DC")
        _scope = "conducted";

    if (_scope.empty() && _target == "SIC")
        _scope = "allocated";

    if (_scope != "conducted" && _scope != "scheduled" &&
        _scope != "allocated" && _scope != "all")
        return false;

    if (_target == "SIC" &&
        _scope != "allocated" && _scope != "all")
        return false;

    if (_target == "DC" &&
        _scope != "conducted" && _scope != "scheduled" && _scope != "all")
        return false;

    return true;
}

void CoreFunctionGetMeasurementsList::handle_sic()
{
    Glib::ustring ret;

    try {
        auto id_mapper = IdMapper::get_instance();
        if (_scope == "allocated")
            ret = id_mapper->get_all_allocated_mappings();
        else
            ret = id_mapper->get_all_mappings();
    } catch (const std::exception& ex) {
        XML_RESULT_INTERNAL_DEVICE_ERROR(ex.what());
    }

    XML_RESULT_OK_RET("", { ret });
}

void CoreFunctionGetMeasurementsList::handle_dc()
{
    auto dc = QueryClient::get_instance();
    auto xq = XmlQueryGetMeasurementsList::create(_scope);
    _query = dc->create_query(xq);
    _query->finished.connect(
        sigc::mem_fun(*this, &CoreFunctionGetMeasurementsList::on_query_finish));
    dc->execute(_query);
}

void CoreFunctionGetMeasurementsList::on_query_finish(const Glib::RefPtr<XmlResult>& result)
{
    call_finished(result);
}

void CoreFunctionGetMeasurementsList::start_call()
{
    std::string dir;

    if (!check_xml_and_get_values()) {
        XML_RESULT_BAD_REQUEST("Malformed XML received");
        return;
    }

    if (_target == "SIC")
        handle_sic();
    else
        handle_dc();
}
