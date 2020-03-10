#include <stdexcept>
#include <iostream>
#include <vector>

#include "xml_string_parameter.h"
#include "xml_file.h"
#include "xml_measurement.h"
#include "xml_result_ok.h"
#include "xml_result_bad_request.h"
#include "xml_result_not_found.h"
#include "xml_result_forbidden.h"
#include "xml_result_internal_device_error.h"
#include "file_handler.h"
#include "conf_handler.h"
#include "id_mapper.h"
#include "utils.h"

#include "core_function_data_free.h"

CoreFunctionDataFree::CoreFunctionDataFree(
    XmlParameterList parameters,
    Glib::RefPtr <XmlDescription> description,
    const Glib::ustring & text)
    : CoreFunctionCall ("dataFree", parameters, description, text)
{ }

CoreFunctionDataFree::~CoreFunctionDataFree ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionDataFree::factory(XmlParameterList parameters,
                              Glib::RefPtr <XmlDescription> description,
                              const Glib::ustring & text,
			      const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr<CoreFunctionCall>(
        new CoreFunctionDataFree(parameters, description, text));
}

void CoreFunctionDataFree::set_deletion_flag() const
{
    std::string path{_measurement_dir};

    path += "/";
    path += _iid;
    path += "/.delete";

    FileHandler::set_file(path, "To be deleted");
}

void CoreFunctionDataFree::free_iid_mapping(const std::string& iid) const
{
    try {
        auto id_mapper = IdMapper::get_instance();
        id_mapper->remove_mapping(iid);
    } catch (const std::exception& ex) {
        PRINT_ERROR("Failed to remove mapping: " << ex.what());
    }
}

void CoreFunctionDataFree::immediate_deletion() const
{
    std::string path{_measurement_dir};

    path += "/";

    // two cases: iid given -> remove that dir, else remove all dirs...
    if (!_iid.empty()) {
        path += _iid;
        FileHandler::remove_directory(path);
        free_iid_mapping(_iid);
    } else {
        auto measurements = FileHandler::list_directory(path);
        for (auto&& measurement : measurements) {
            FileHandler::remove_directory(path + measurement);
            free_iid_mapping(measurement);
        }
    }
}

void CoreFunctionDataFree::start_call()
{
    auto conf_handler = ConfHandler::get_instance();
    _measurement_dir  = conf_handler->getDirectory("measurements");

    if (_measurement_dir.empty()) {
        XML_RESULT_INTERNAL_DEVICE_ERROR("Failed to get measurement directory");
        return;
    }

    auto iid_param = _parameters.get<XmlStringParameter>("iid");
    auto enforce_param = _parameters.get<XmlStringParameter>("enforceDeletion");
    const bool enforce_deletion = (enforce_param && enforce_param->get_str() == "yes") ? true : false;

    if (!iid_param && !enforce_deletion) {
        XML_RESULT_BAD_REQUEST("Received malformed dataFree request");
        return;
    }

    _iid = iid_param ? iid_param->get_str() : "";

    try {
        if (enforce_deletion)
            immediate_deletion();
        else
            set_deletion_flag();
    } catch (const std::exception& ex) {
        XML_RESULT_NOT_FOUND("Could not flag measurement " << _iid << " for deletion");
        return;
    }

    XML_RESULT_OK("");
}
