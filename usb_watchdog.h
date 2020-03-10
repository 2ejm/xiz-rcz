#ifndef _USB_WATCHDOG_H_
#define _USB_WATCHDOG_H_

#include <glibmm/object.h>
#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <giomm/dbusconnection.h>
#include <giomm/dbusproxy.h>

#include <map>
#include <string>

#include "watchdog.h"

#include "udisks_device.h"
#include "state_variable.h"

/**
 * \brief USBWatchDog.
 *
 * Handles USB folder input only once at plugin. This watchdog works like the
 * LAN watchdog with one exception: It scans the input folder recursively.
 */
class UsbWatchdog final : public Watchdog
{
public:
    using DeviceMap = std::map<Glib::ustring, Glib::RefPtr<UdisksDevice> >;

    virtual ~UsbWatchdog()
    {}

    static inline Glib::RefPtr<UsbWatchdog> create (const std::string& input)
    {
        return Glib::RefPtr<UsbWatchdog>(new UsbWatchdog(input));
    }

    const std::string& get_current_mount_path() const
    {
        return _mount_path;
    }

    // Read connected usb sticks manualy; Has to be called on startup
    void rescan();

private:
    Glib::RefPtr<Gio::DBus::Connection> _bus;
    Glib::RefPtr<Gio::DBus::Proxy> _udisks_proxy;
    DeviceMap _devices;

    /* There is only a single current device
     * We only need to save the mount_path and the device name
     */
    std::string _mount_path;
    Glib::ustring _current_device;

    StateVariable _usb_state;

    UsbWatchdog (const std::string& input);

    LanFolderList create_file_list_rec(const std::string& folder) const;

    void setup_dbus();

    /**
     * That's the function called once upon a usb stick was inserted and
     * mounted. In difference to the lan_watchdog handler, this function works
     * recursively.
     */
    bool usb_handler(bool update_only = false);

    bool timeout_handler();

    sigc::connection _rescan_timer;

    void udisks_signal_dispatcher(
        const Glib::ustring& sender_name, const Glib::ustring& signal_name,
        const Glib::VariantContainerBase& parameters);

    void udisks_device_added(const Glib::ustring& device);
    void udisks_device_changed(const Glib::ustring& device);
    void udisks_device_removed(const Glib::ustring& device);

    bool is_first_partition(const Glib::ustring& device);
    bool part_belongs_to_disk(const Glib::ustring& device);

    const std::vector<std::string> get_input_folder_paths(bool update_only = false) const;

    ZixInterface get_zix_interface() const override
    {
        return STR_ZIXINF_USB;
    }

    FileDestination get_file_destination() const noexcept override
    {
        return FileDestination::Dest::usb;
    }

    std::string get_root_folder() const override
    {
        return _mount_path;
    }
};

#endif /* _USB_WATCHDOG_H_ */
