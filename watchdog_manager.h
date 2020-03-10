#ifndef _WATCHDOG_MANAGER_H_
#define _WATCHDOG_MANAGER_H_

#include <glibmm/object.h>
#include <glibmm/refptr.h>

#include <vector>

#include "watchdog.h"
#include "lan_watchdog.h"
#include "usb_watchdog.h"
#include "conf_handler.h"

static const std::string empty_string = "";

/**
 * \brief Manages all WatchDogs.
 */
class WatchdogManager : public Glib::Object
{
public:
    using LanWatchdogList = std::vector<Glib::RefPtr<Watchdog> >;
    using RefPtr = Glib::RefPtr<WatchdogManager>;

    static inline RefPtr& get_instance()
    {
        if (!instance)
            instance = create();
        return instance;
    }

    void create_lan_watchdogs_by_config();
    void create_usb_watchdog_by_config();

    inline auto size() const noexcept -> LanWatchdogList::size_type
    {
        return _lan_watchdogs.size();
    }

    inline void activate() noexcept
    {
        for (auto&& w : _lan_watchdogs)
            w->activate();
    }

    inline void deactivate() noexcept
    {
        for (auto&& w : _lan_watchdogs)
            w->deactivate();
    }

    const std::string& usb_mount_path() const
    {
        if (!_usb_watchdog)
            return empty_string;
        return _usb_watchdog->get_current_mount_path();
    }

    Glib::RefPtr<UsbWatchdog> get_usb_watchdog()
    {
        return(_usb_watchdog);
    }

private:
    static RefPtr instance;

    LanWatchdogList _lan_watchdogs;
    Glib::RefPtr<UsbWatchdog> _usb_watchdog;
    Glib::ustring _current_lan_input_folder;
    Glib::ustring _current_lan_input_timer;

    WatchdogManager() :
        Glib::Object()
    {
        auto conf_handler = ConfHandler::get_instance();

        // connect to config changed signal
        conf_handler->confChangeAnnounce.connect(
            sigc::mem_fun(*this, &WatchdogManager::on_config_changed_announce));
        conf_handler->confChanged.connect(
            sigc::mem_fun(*this, &WatchdogManager::on_config_changed));
    }

    static inline RefPtr create()
    {
        return RefPtr(new WatchdogManager());
    }

    void on_config_changed_announce(const Glib::ustring& par_id, const Glib::ustring& value, int& handler_mask);
    void on_config_changed(const int handlerMask);
};

#endif /* _WATCHDOG_MANAGER_H_ */
