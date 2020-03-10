#include <vector>

#include "file_handler.h"
#include "utils.h"
#include "network_config.h"

#include "samba_mounter.h"

SambaMounter::RefPtr SambaMounter::instance(nullptr);

SambaMounter::SambaMounter()
    : Glib::Object()
    , _is_mounted{false}
    , _mount_path{"/media/lan_folder"}
    , _samba_state ("statusLanFolder")
{
    ConfHandler::get_instance()->confChangeAnnounce.connect(
	    sigc::mem_fun(*this, &SambaMounter::on_conf_change_announce));
    ConfHandler::get_instance()->confChanged.connect(
	    sigc::mem_fun(*this, &SambaMounter::on_conf_changed));
}

void SambaMounter::mount()
{
    if (_is_mounted)
        return;

    // create mount directory
    try {
        FileHandler::create_directory(_mount_path);
    } catch (const std::exception& ex) {
        PRINT_ERROR("Failed to create samba mount directory");
	_samba_state.set_state ("nodevice");
        mount_ready.emit(false);
        return;
    }

    auto conf_handler  = ConfHandler::get_instance();
    auto enabled       = conf_handler->getParameter("lanFolder");
    auto share         = conf_handler->getPath("lanFolderShare");
    auto user          = conf_handler->getParameter("lanFolderUser");
    auto password      = conf_handler->getParameter("lanFolderPassword");
    auto extra_options = conf_handler->getParameter("lanFolderMountingOptions");

    _share = share;
    _user  = user;
    _pass  = password;

    if (enabled != "enabled") {
        PRINT_INFO("Not mounting shared folder. It's not enabled.");
	_samba_state.set_state ("nodevice");
        mount_ready.emit(false);
        return;
    }

    if (share == "") {
        PRINT_INFO("No shared folder share given. Not mounting.");
	_samba_state.set_state ("nodevice");
        mount_ready.emit(false);
        return;
    }

    if (user == "") {
        PRINT_INFO("No user for shared folder given. Not mounting.");
	_samba_state.set_state ("nodevice");
        mount_ready.emit(false);
        return;
    }

    auto net = CNetworkConfig::get_instance ();
    if (!net->online ()) {
	_samba_state.set_state ("nodevice");
        mount_ready.emit(false);
	return;
    }

    try {
        std::string ops;
        std::stringstream ss;
        // note: we do a group id lookup due to lack of mount.cifs on target
        auto www_data_id = FileHandler::get_groupd_id("www-data");

        ss << "username=" << user << ",password=" << password;
        ss << ",uid=" << www_data_id << ",gid=" << www_data_id;
        ss << ",file_mode=0770,dir_mode=0770";
        if (!extra_options.empty())
            ss << "," << extra_options;
        ops = ss.str();
        std::vector<std::string> mount_args{ "mount", "-t", "cifs", "-o", ops,
                share, _mount_path };

        _mount_proc = ProcessRequest::create(mount_args, ProcessRequest::DEFAULT_TIMEOUT);
        _mount_proc->finished.connect(
            sigc::mem_fun(*this, &SambaMounter::on_mount_finished));
        _mount_proc->start_process();
    } catch (const std::exception& ex) {
        PRINT_ERROR("Failed to start samba mount process: " << ex.what());
	_samba_state.set_state ("nodevice");
    }
}

void SambaMounter::umount(bool mount_after)
{
    try {
        std::vector<std::string> mount_args{ "umount", _mount_path };

        _umount_proc = ProcessRequest::create(mount_args, ProcessRequest::DEFAULT_TIMEOUT);
        _umount_proc->finished.connect(
            sigc::bind<bool>(
                sigc::mem_fun(*this, &SambaMounter::on_umount_finished),
                mount_after));
        _umount_proc->start_process();
    } catch (const std::exception& ex) {
        PRINT_ERROR("Failed to start samba umount process: " << ex.what());
    }
}

void SambaMounter::on_mount_finished(const Glib::RefPtr<ProcessResult>& result)
{
    if (!result->success()) {
        PRINT_ERROR("Samba mount failed: " << result->error_reason());
	_samba_state.set_state ("nodevice");
        mount_ready.emit(false);
        return;
    }

    _is_mounted = true;
    _samba_state.set_state ("mountedrw");
    mount_ready.emit(true);
}

void SambaMounter::on_umount_finished(
    const Glib::RefPtr<ProcessResult>& result, bool mount_after)
{
    if (!result->success()) {
        PRINT_ERROR("Samba umount failed: " << result->error_reason());
        return;
    }

    _is_mounted = false;
    _samba_state.set_state ("unmounted");

    if (mount_after)
        mount();
}

void SambaMounter::remount()
{
    umount(true);
}

void SambaMounter::on_conf_change_announce(
    const Glib::ustring& par_id, const Glib::ustring& value, int& handler_mask)
{
    UNUSED(value);

    if (par_id == "lanFolder" ||
        par_id == "lanFolderShare" ||
        par_id == "lanFolderUser" ||
        par_id == "all")
        handler_mask |= HANDLER_MASK_SAMBA;
}

void SambaMounter::on_conf_changed(const int handler_mask)
{
    if (!(handler_mask & HANDLER_MASK_SAMBA))
        return;

    auto conf_handler = ConfHandler::get_instance();
    auto enabled      = conf_handler->getBoolParameter("lanFolder");

    // check enable/disable
    if (enabled && !_is_mounted) {
        mount();
        return;
    }
    if (!enabled && _is_mounted) {
        umount();
        return;
    }

    // check parameter for changes
    if (_is_mounted) {
        auto share    = conf_handler->getPath("lanFolderShare");
        auto user     = conf_handler->getParameter("lanFolderUser");
        auto password = conf_handler->getParameter("lanFolderPassword");

        if (_share != share ||
            _user  != user ||
            _pass  != password) {
            PRINT_DEBUG("Detected changed LAN folder settings. Remounting.");
            remount();
        }
    }
}
