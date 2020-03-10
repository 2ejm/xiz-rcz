#include <string>
#include <sstream>

#include "disk_usage_manager.h"

#include "file_handler.h"
#include "log_handler.h"
#include "conf_handler.h"
#include "monitor_manager.h"
#include "id_mapper.h"
#include "utils.h"

DiskUsageManager::RefPtr DiskUsageManager::instance(nullptr);

DiskUsageManager::DiskUsageManager() :
    Glib::Object(),
    _number_of_measurements_max{100}, _number_of_measurements_keep{5},
    _number_of_log_max{20}, _number_of_log_keep{5}
{
    update_config();
    auto conf_handler = ConfHandler::get_instance();
    conf_handler->confChangeAnnounce.connect(
        sigc::mem_fun(*this, &DiskUsageManager::on_config_change_announce));
    conf_handler->confChanged.connect(
        sigc::mem_fun(*this, &DiskUsageManager::on_config_changed));
}

void DiskUsageManager::update_config()
{
    get_config_value("numberOfMeasurementsMax");
    get_config_value("numberOfMeasurementsKeep");
    get_config_value("numberOfLogMax");
    get_config_value("numberOfLogKeep");
}

void DiskUsageManager::remove_flagged_measurements() const
{
    try {
        auto conf_handler = ConfHandler::get_instance();
        std::string path  = conf_handler->getDirectory("measurements");
        if (path.empty())
            return;

        auto dir = FileHandler::list_directory(path);
        auto nr_of_measurements = dir.size();

        for (auto&& file : dir) {
            auto entry = path + "/" + file;

            if (nr_of_measurements <= _number_of_measurements_keep)
                break;

            if (!FileHandler::directory_exists(entry))
                continue;

            if (!FileHandler::file_exists(entry + "/.delete"))
                continue;

            FileHandler::remove_directory(entry);
            --nr_of_measurements;

            try {
                IdMapper::get_instance()->remove_mapping(file);
            } catch (const std::exception& ex) {
                PRINT_ERROR("Failed to remove id mapping for " << file);
            }
        }
    } catch (const std::exception& ex) {
        PRINT_ERROR("Error ocurred while removing flagged measurements: " << ex.what());
    }
}

void DiskUsageManager::logs_truncate() const
{
    try {
        auto logger = LogHandler::get_instance();
        auto monitor_manager = MonitorManager::get_instance();
        logger->log_truncate(_number_of_log_keep);
        monitor_manager->log_truncate(_number_of_log_keep);
    } catch (const std::exception& ex) {
        PRINT_ERROR("Error ocurred while truncating log file: " << ex.what());
    }
}
void DiskUsageManager::get_config_value(const std::string& name)
{
    int val;
    std::stringstream ss;
    std::string param;

    auto conf_handler = ConfHandler::get_instance();
    param = conf_handler->getParameter(name);

    if (param.empty())
        return;

    ss.str(param);
    if (!(ss >> val))
        return;

    if (name == "numberOfMeasurementsMax")
        _number_of_measurements_max = val;
    if (name == "numberOfMeasurementsKeep")
        _number_of_measurements_keep = val;
    if (name == "numberOfLogMax")
        _number_of_log_max = val;
    if (name == "numberOfLogKeep")
        _number_of_log_keep = val;
}

void DiskUsageManager::on_config_change_announce(
    const Glib::ustring& par_id, const Glib::ustring& value, int &handlerMask)
{
    (void)value;
    if (    ( par_id == "numberOfMeasurementsMax")
         || ( par_id == "numberOfMeasurementsKeep")
         || ( par_id == "numberOfLogMax" )
         || ( par_id == "numberOfLogKeep")
	 || ( par_id == "all" ))
        handlerMask |= HANDLER_MASK_DU_MANAGER;
}


void DiskUsageManager::on_config_changed( const int handlerMask)
{
    if ( handlerMask & HANDLER_MASK_DU_MANAGER )
        update_config();
}
