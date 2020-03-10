
#include "core_function_call.h"

#include "xml_exception.h"
#include "xml_string_parameter.h"
#include "file_handler.h"

#include "core_function_log.h"
#include "core_function_get_files_list.h"
#include "core_function_del_file.h"
#include "core_function_get_file.h"
#include "core_function_set_file.h"
#include "core_function_get_conf.h"
#include "core_function_set_conf.h"
#include "core_function_data_sync.h"
#include "core_function_data_out.h"
#include "core_function_data_free.h"
#include "core_function_data_in.h"
#include "core_function_wrap_dc.h"
#include "core_function_get_measurement.h"
#include "core_function_get_measurements_list.h"
#include "core_function_procedure.h"
#include "core_function_exit.h"
#include "core_function_update.h"

CoreFunctionCall::CoreFunctionCall (const Glib::ustring & fid,
				    XmlParameterList parameters,
				    Glib::RefPtr <XmlDescription> description,
				    const Glib::ustring & textbody)
    : _parameters (parameters)
    , _description (description)
    , _textbody (textbody)
    , _fid (fid)
{ }

CoreFunctionCall::~CoreFunctionCall()
{ }

Glib::RefPtr<FunctionCall>
CoreFunctionCall::create (const Glib::ustring & fid,
			  XmlParameterList parameters,
			  Glib::RefPtr <XmlDescription> description,
			  const Glib::ustring & textbody,
			  const xmlpp::Element *en,
			  const Glib::ustring & interface,
                          const Glib::ustring & filename)
{
    auto f_iter = factory_map.find (fid);

    /* check, whether we have an entry here
     */
    if (f_iter == factory_map.end()) {
	throw XmlExceptionUnknownFid (fid);
    }

    /* ok... call constructor and return result
     */
    auto ret = f_iter->second (parameters, description, textbody, en);

    ret->set_interface(interface);
    ret->set_filename(filename);

    return Glib::RefPtr <FunctionCall>::cast_dynamic (ret);
}

void
CoreFunctionCall::call_finished (Glib::RefPtr <XmlResult> result)
{
    PRINT_DEBUG ("CoreFunctionCall::call_finished");

    auto dest_param = _parameters.get<XmlStringParameter>("dest");

    if (dest_param) {
        PRINT_DEBUG ("dest param is set to " << dest_param->get_str());
        auto dest = dest_param->get_str();
        FileHandler::set_file (dest, result->to_dest ());
	result->set_redirected ();
    }

    finished.emit (result);
}

std::map <Glib::ustring, CoreFunctionCall::Factory> CoreFunctionCall::factory_map;

void
CoreFunctionCall::register_factory (const Glib::ustring & fid, Factory factory)
{
    factory_map [fid] = factory;
}

void CoreFunctionCall::set_interface(const Glib::ustring & interface)
{
	_interface = interface;
}

void CoreFunctionCall::set_filename(const Glib::ustring & filename)
{
	_filename = filename;
}

void
CoreFunctionCall::init ()
{
    register_factory ("log", & CoreFunctionLog::factory);
    register_factory ("getFilesList", & CoreFunctionGetFilesList::factory);
    register_factory ("delFile", & CoreFunctionDelFile::factory);
    register_factory ("getFile", & CoreFunctionGetFile::factory);
    register_factory ("setFile", & CoreFunctionSetFile::factory);
    register_factory ("getConf", & CoreFunctionGetConf::factory);
    register_factory ("setConf", & CoreFunctionSetConf::factory);
    register_factory ("dataSync", & CoreFunctionDataSync::factory);
    register_factory ("dataOut", & CoreFunctionDataOut::factory);
    register_factory ("dataFree", & CoreFunctionDataFree::factory);
    register_factory ("getMeasurement", & CoreFunctionGetMeasurement::factory);
    register_factory ("dataIn", & CoreFunctionDataIn::factory);
    register_factory ("getMeasurementsList", &CoreFunctionGetMeasurementsList::factory);
    register_factory ("update", &CoreFunctionUpdate::factory);

    register_factory ("setMeasurementsList", CoreFunctionWrapDC::factory_set_measurements_list);
    register_factory ("getCalibration", CoreFunctionWrapDC::factory_get_calibration);
    register_factory ("getDefaults", CoreFunctionWrapDC::factory_get_defaults);
    register_factory ("getProfilesList", CoreFunctionWrapDC::factory_get_profiles_list);
    register_factory ("getProfile", CoreFunctionWrapDC::factory_get_profile);
    register_factory ("getParametersList", CoreFunctionWrapDC::factory_get_parameters_list);
    register_factory ("getParameter", CoreFunctionWrapDC::factory_get_parameter);
    register_factory ("getTemplatesList", CoreFunctionWrapDC::factory_get_templates_list);
    register_factory ("getTemplate", CoreFunctionWrapDC::factory_get_template);
    register_factory ("setMeasurement", CoreFunctionWrapDC::factory_set_measurement);
    register_factory ("setCalibration", CoreFunctionWrapDC::factory_set_calibration);
    register_factory ("setDefaults", CoreFunctionWrapDC::factory_set_defaults);
    register_factory ("setProfilesList", CoreFunctionWrapDC::factory_set_profiles_list);
    register_factory ("setProfile", CoreFunctionWrapDC::factory_set_profile);
    register_factory ("setParameter", CoreFunctionWrapDC::factory_set_parameter);
    register_factory ("setTemplatesList", CoreFunctionWrapDC::factory_set_templates_list);
    register_factory ("setTemplate", CoreFunctionWrapDC::factory_set_template);
    register_factory ("delMeasurement", CoreFunctionWrapDC::factory_del_measurement);
    register_factory ("delProfile", CoreFunctionWrapDC::factory_del_profile);
    register_factory ("delTemplate", CoreFunctionWrapDC::factory_del_template);
    register_factory ("guiIO", CoreFunctionWrapDC::factory_gui_io);

    register_factory ("procedure", CoreFunctionProcedure::factory);
    register_factory ("exit", CoreFunctionExit::factory);
}

