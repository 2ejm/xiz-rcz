#include "conf_handler.h"
#include "file_destination.h"
#include "utils.h"

#include "watchdog_manager.h"

WatchdogManager::RefPtr WatchdogManager::instance(nullptr);

void WatchdogManager::create_lan_watchdogs_by_config()
{
    auto conf_handler   = ConfHandler::get_instance();

    // gather configurations
    FileDestination lan_dest(FileDestination::Dest::lan);
    auto lan_input_folder   = conf_handler->getInputFolder(lan_dest, "");

    // setup lan watchdogs
    for (auto&& folder : lan_input_folder) {
        // check for timer
        if (folder.timer().value() == "0")
            continue;

        _lan_watchdogs.emplace_back(
            LanWatchdog::create(folder.path(), folder.timer()));
    }

    // save current lan folder settings
    _current_lan_input_folder = conf_handler->getParameter("lanFolderInputPath");
    _current_lan_input_timer  = conf_handler->getParameter("lanFolderInputTimer");
}

void WatchdogManager::create_usb_watchdog_by_config()
{
    //
    // Setup usb watch dog:
    //  - There is only one
    //  - Input and log folder are always fetch via zixconf.xml
    //
    _usb_watchdog = UsbWatchdog::create ("/");
}

void WatchdogManager::on_config_changed_announce(
    const Glib::ustring& par_id, const Glib::ustring& value, int& handler_mask)
{
    UNUSED(value);

    if (par_id == "lanFolder" ||
        par_id == "usbFolder" ||
        par_id == "lanFolderInputPath" ||
        par_id == "lanFolderInputTimer" ||
        par_id == "all")
        handler_mask |= HANDLER_MASK_WATCHDOG_MANAGER;
}

void WatchdogManager::on_config_changed(const int handler_mask)
{
    if (!(handler_mask & HANDLER_MASK_WATCHDOG_MANAGER))
        return;

    auto conf_handler = ConfHandler::get_instance();

    // Enable/disable watchdogs?
    auto lan_enabled = conf_handler->getBoolParameter("lanFolder");
    auto usb_enabled = conf_handler->getBoolParameter("usbFolder");

    if (_lan_watchdogs.size() > 0 && lan_enabled != _lan_watchdogs[0]->activated()) {
        for (auto&& w : _lan_watchdogs)
            w->activated() = lan_enabled;
        PRINT_DEBUG("" << (lan_enabled ? "Enabled" : "Disabled") << " LAN Watchdogs");
    }

    if (usb_enabled != _usb_watchdog->activated()) {
        _usb_watchdog->activated() = usb_enabled;
        PRINT_DEBUG("" << (usb_enabled ? "Enabled" : "Disabled") << " USB Watchdog");
    }

    // Reconfigure LAN watchdogs?
    auto lan_input_folder = conf_handler->getParameter("lanFolderInputPath");
    auto lan_input_timer  = conf_handler->getParameter("lanFolderInputTimer");

    if (_current_lan_input_folder != lan_input_folder ||
        _current_lan_input_timer != lan_input_timer) {
        PRINT_DEBUG("Reconfigure LAN watchdogs");
        _lan_watchdogs.clear();
        create_lan_watchdogs_by_config();
    }
}
