#include <glibmm/main.h>

#include <sigc++/signal.h>

#include <stdexcept>
#include <regex>
#include <algorithm>

#include "udisks_device.h"
#include "file_handler.h"
#include "utils.h"
#include "conf_handler.h"

#include "usb_watchdog.h"

UsbWatchdog::UsbWatchdog( const std::string& input)
    : Watchdog ("usbFolder",
	        input,
		ConfHandler::get_instance ()->getBoolParameter ("usbFolder"))
    , _usb_state ("statusUsbFolder")
{
    setup_dbus();
    PRINT_DEBUG("Usb Watchdog ready. Listening on Udisks DBus events. actived=" << _activated);
}

UsbWatchdog::LanFolderList UsbWatchdog::create_file_list_rec(const std::string& folder) const
{
    LanFolderList result = create_file_list(folder);
    auto files = FileHandler::list_directory(folder);
    PRINT_DEBUG("UsbWatchdog::create_file_list_rec looking at folder " << folder);

    for (auto&& file : files) {
        try {
            auto new_path = folder + "/" + file;

            // sanity checks first
            if (!FileHandler::directory_exists(new_path))
                continue;
            // exclude log dir from recursive runs
            if (new_path == this->get_root_folder() + "/" + get_log_folder())
                continue;

            auto sub_files = create_file_list(new_path);
            result.insert(result.end(), sub_files.begin(), sub_files.end());
        } catch (const std::exception& ex) {
            // ignore -> might be permission denied or sth. like that
            PRINT_ERROR(ex.what());
        }
    }

    return result;
}

void UsbWatchdog::setup_dbus()
{
    // setup
    _bus          = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SYSTEM);
    _udisks_proxy = Gio::DBus::Proxy::create_sync(
        _bus, "org.freedesktop.UDisks", "/org/freedesktop/UDisks", "org.freedesktop.UDisks");

    // connect signal handler
    _udisks_proxy->signal_signal().connect(
        sigc::mem_fun(*this, &UsbWatchdog::udisks_signal_dispatcher));
}

void UsbWatchdog::rescan()
{
    Glib::VariantContainerBase devices_variant = _udisks_proxy->call_sync("EnumerateDevices");
    Glib::VariantIter iterator(devices_variant.get_child(0));

    lHighDebug ("UsbWatchdog::rescan() is called\n");

    Glib::Variant<Glib::ustring> var;
    while(iterator.next_value(var)) {
        Glib::ustring name = var.get();

        lInfo("Found Device: '%s", name.c_str());
        udisks_device_added(name);
    }

    /* when no device is mounted, we set the state here.
     * the state has been set in udisks_device_added()
     * when a device is mounted
     */
    if (_current_device.empty ()) {
	_usb_state.set_state ("nodevice");
    }
}

void UsbWatchdog::udisks_signal_dispatcher(
        const Glib::ustring& sender_name, const Glib::ustring& signal_name,
        const Glib::VariantContainerBase& parameters)
{
    UNUSED(sender_name);

    Glib::Variant<Glib::ustring> device;
    parameters.get_child(device, 0);

    try {
        lDebug("Udisks Signal: %s\n", signal_name.c_str());
        if (signal_name == "DeviceAdded")
            udisks_device_added(device.get());
        if (signal_name == "DeviceChanged")
            udisks_device_changed(device.get());
        if (signal_name == "DeviceRemoved")
            udisks_device_removed(device.get());
    } catch (const std::exception& ex) {
        PRINT_ERROR(ex.what());
    } catch (const Glib::Error& ex) {
        PRINT_ERROR(ex.what());
    }
}

bool UsbWatchdog::is_first_partition(const Glib::ustring& device)
{
    const auto& dev     = _devices[device]->get("DeviceFile");
    const auto& is_part = _devices[device]->get("DeviceIsPartition");

    PRINT_DEBUG ("UsbWatchdog::is_first_partition dev=" << dev << " is_part=" << is_part);
    if (is_part == "1") {
        std::regex re("[A-Za-z/]+(\\d+)");
        std::smatch match;

        if (std::regex_match(dev.raw(), match, re)) {
            std::string num = match[1];
	    PRINT_DEBUG ("UsbWatchdog::is_first_partition checking " << num);
            return num == "1";
        }
    }

    return false;
}

bool UsbWatchdog::part_belongs_to_disk(const Glib::ustring& device)
{
    const auto& connection = _devices[device]->get("DriveConnectionInterface");

    return connection == "usb";
}

void UsbWatchdog::udisks_device_added(const Glib::ustring& device)
{
    PRINT_DEBUG ("in UsbWatchdog::udisks_device_added");
    // add new device
    _devices[device] = UdisksDevice::create(_bus, device);

    // if not USB, then it's not for me
    if (_devices[device]->get("DriveConnectionInterface") != "usb" ||
        _devices[device]->get("DeviceIsSystemInternal") == "1") {
	PRINT_DEBUG ("Device not suitable, calling udisks_device_removed()");
        udisks_device_removed(device);
        return;
    }

    PRINT_DEBUG("Device " << device << " added");

    // we only take the first partition for now
    if (is_first_partition(device) && part_belongs_to_disk(device)) {
	/* a suitable device was plugged.
	 */
        PRINT_DEBUG("Found suitable USB stick @ " << _devices[device]->get("DeviceFile"));

	/* if we already have a "current" device, we just return
	 */
	if (!_current_device.empty ())
	    return;

	/* remember the current device
	 */
	_current_device = device;

        if (_devices[device]->get("DeviceIsMounted") == "0")
            _mount_path = _devices[device]->mount();
        else
            _mount_path = _devices[device]->get("DeviceMountPaths");

        /*
         * Check read-only case
         */
        if (_devices[device]->get("DeviceIsReadOnly") == "1") {
            PRINT_ERROR("USB Stick is read-only. Try to remount it rw.");

            _usb_state.set_state ("mountedro");

            /* remount rw */
            _devices[device]->unmount();
            _mount_path = _devices[device]->mount();

            /* update props */
            _devices[device]->update_properties();

            /* recheck */
            if (_devices[device]->get("DeviceIsReadOnly") == "1") {
                PRINT_ERROR("USB Stick is still read-only. Not running USB handler.");
                return;
            }
        }

        _usb_state.set_state ("mountedrw");

        PRINT_DEBUG("Mount path for USB stick \"" << _mount_path << "\". Running handler.");

        _rescan_timer = Glib::signal_timeout().connect(sigc::mem_fun(*this, &UsbWatchdog::timeout_handler), 60 * 1000);

        usb_handler();
    }
}

void UsbWatchdog::udisks_device_changed(const Glib::ustring& device)
{
    // update device properties
    auto it = _devices.find(device);
    if (it != _devices.end()) {
        PRINT_DEBUG("Device " << device << " changed. Updating properties.");
        _devices[device]->update_properties();
    }
}

void UsbWatchdog::udisks_device_removed(const Glib::ustring& device)
{
    /* check, whether this is the "current" device
     */

    if (_current_device == device) {
	/* device removed, clear mount path and current device
	 */
	_current_device.clear ();
	_mount_path.clear ();
	_usb_state.set_state ("nodevice");
        if (_rescan_timer.connected())
            _rescan_timer.disconnect();
    }

    // remove device
    auto it = _devices.find(device);
    if (it != _devices.end()) {
        PRINT_DEBUG("Device " << device << " removed");
        _devices.erase(device);
    }
}

const std::vector<std::string> UsbWatchdog::get_input_folder_paths(bool update_only) const
{
    std::vector<std::string> result;
    auto conf_handler = ConfHandler::get_instance();
    FileDestination usb_dest(FileDestination::Dest::usb);
    auto usb_folders = conf_handler->getInputFolder(usb_dest, update_only ? "update" : "");
    for (auto&& usb_folder : usb_folders)
        result.push_back(usb_folder.path());

    return result;
}

bool UsbWatchdog::usb_handler(bool update_only)
{
    if (!_activated || _in_progress) {
	lDebug ("Usb handler called, but exiting due to (!_activated=%d || _in_progress=%d)\n", (int)_activated, (int) _in_progress);
        return true;
    }

    try {
        //  prepend _input_folder with current the mount folder
        LanFolderList tmp_folder_list;
        auto input_folders = get_input_folder_paths(update_only);

        _folder_list.clear();
        for (auto&& input_folder : input_folders) {
            tmp_folder_list = create_file_list_rec(this->get_root_folder() + "/" + input_folder);
            _folder_list.insert(_folder_list.end(), tmp_folder_list.begin(), tmp_folder_list.end());
        }

        sort_file_list(_folder_list);

        if (_folder_list.empty())
            return true;

        _in_progress = true;
        _folder_it = _folder_list.cbegin();
        process_entry(*_folder_it);
    } catch (const std::exception& ex) {
        PRINT_ERROR(ex.what());
        _in_progress = false;
    }

    return true;
}

bool UsbWatchdog::timeout_handler()
{
    if (!_current_device.empty() && !_in_progress)
        usb_handler(true);
    return true;
}
