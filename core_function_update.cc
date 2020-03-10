#include <glibmm/ustring.h>
#include <glibmm/miscutils.h>

#include <stdexcept>
#include <vector>
#include <string>

#include "xml_string_parameter.h"
#include "xml_query_set_file.h"
#include "xml_query_set_parameter.h"
#include "file_handler.h"
#include "utils.h"
#include "usb_watchdog.h"           // store update xml on usb
#include "watchdog_manager.h"       // store update xml on usb
#include "samba_mounter.h"
#include "state_variable.h"
#include "zix_interface.h"
#include "update_signature_check_request.h"
#include "signature_check_request.h"
#include "network_config.h"

#include "xml_image.h"

#include "core_function_update.h"

#define DC_BOOT_TIMEOUT (20)

CoreFunctionUpdate::CoreFunctionUpdate(
    XmlParameterList parameters,
    Glib::RefPtr <XmlDescription> description,
    const Glib::ustring& text)
    : CoreFunctionCall ("update", parameters, description, text)
{ }

CoreFunctionUpdate::~CoreFunctionUpdate ()
{ }

Glib::RefPtr <CoreFunctionCall>
CoreFunctionUpdate::factory(XmlParameterList parameters,
                            Glib::RefPtr <XmlDescription> description,
                            const Glib::ustring & text,
                            const xmlpp::Element * en)
{
    (void) en;

    return Glib::RefPtr<CoreFunctionCall>(
        new CoreFunctionUpdate(parameters, description, text));
}

bool CoreFunctionUpdate::check_xml_and_get_values()
{
    const auto target_param = _parameters.get<XmlStringParameter>("target");
    const auto method_param = _parameters.get<XmlStringParameter>("method");

    _image_param  = _parameters.get<XmlImage>("image");
    if (!_image_param) {
        XML_RESULT_BAD_REQUEST ("Invalid or missing image parameter");
        return false;
    }

    if (_interface != STR_ZIXINF_USB && _interface != STR_ZIXINF_LANSHAREDFOLDER) {
        XML_RESULT_BAD_REQUEST("Update only from interface USB or LAN:sharedFolder");
        return false;
    }

    if (_image_param->get_type () == "file") {
        /* type="file" means usb for now.
         */
        if (_interface == STR_ZIXINF_USB) {
            _usb_mount_path = WatchdogManager::get_instance()->usb_mount_path();
            if (_usb_mount_path.empty()) {
#if defined GLOBAL_INSTALLATION
                XML_RESULT_INTERNAL_DEVICE_ERROR("No USB stick for updates connected. Is the USB Watchdog enabled ?" );
                return false;
#else
                _usb_mount_path="/tmp";
#endif
            }
        }else {
            /*interface == LAN:sharedFolder
             */
            _lan_mount_path = SambaMounter::get_instance ()->mount_path ();
            if (_lan_mount_path.empty()) {
                XML_RESULT_INTERNAL_DEVICE_ERROR("No Lan Folder connected, is it activated ?" );
                return false;
            }
        }
    }

    _target = target_param ? target_param->get_str() : "";
    _method = method_param ? method_param->get_str() : "";

    if (_target != "DCBL" && _target != "DCF" &&
        _target != "SICBL" && _target != "SICF" &&
        _target != "SICK" && _target != "SICR"
        ) {
        XML_RESULT_BAD_REQUEST ("Bad target specified");
        return false;
    }

    if (_method == "")
        _method = "SF";

    if (_method != "SF") {
        XML_RESULT_BAD_REQUEST ("Only method SF is supported");
        return false;
    }

    /*
     * Check SIC updates: Currently the sic-updater in the rescue system can
     * only deal with image_type == "file", not "binary". That feature is not
     * implemented, yet and probably not needed.
     *
     * Note: SICBL with type "binary" works.
     */
    if (_image_param->get_type () == "binary" &&
        (_target == "SICF" || _target == "SICK" || _target == "SICR")) {
        XML_RESULT_BAD_REQUEST("Binary update for SICF, SICR and SICK is not implemented");
        return false;
    }

    return true;
}

Glib::ustring
CoreFunctionUpdate::get_lan_settings_xml ()
{
    Glib::ustring ret;

    auto conf = ConfHandler::get_instance ();

    /* collect network settings
     */
    Glib::ustring lan_config_mode = conf->getConfParameter( "lanConfigurationMode" );
    Glib::ustring lan_ip_address  = conf->getConfParameter( "lanIPaddress" );
    Glib::ustring lan_subnet_mask = conf->getConfParameter( "lanSubnetMask" );
    Glib::ustring lan_gateway     = conf->getConfParameter( "lanGateway" );
    Glib::ustring lan_dns1        = conf->getConfParameter( "lanDNS1" );
    Glib::ustring lan_dns2        = conf->getConfParameter( "lanDNS2" );

    /* distinguish dhcp and no dhcp
     * and add them to ret
     */
    if (lan_config_mode == "DHCP") {
        ret += "<lansettings mode=\"DHCP\"/>\n";
    } else {
        ret += Glib::ustring::compose ("<lansettings mode=\"%1\" ip=\"%2\" subnet=\"%3\" gateway=\"%4\" dns1=\"%5\" dns2=\"%6\"/>\n",
                                       lan_config_mode,
                                       lan_ip_address,
                                       lan_subnet_mask,
                                       lan_gateway,
                                       lan_dns1,
                                       lan_dns2);
    }

    /* now collect smb settings
     */
    auto share         = conf->getPath("lanFolderShare");
    auto user          = conf->getParameter("lanFolderUser");
    auto password      = conf->getParameter("lanFolderPassword");
    auto extra_options = conf->getParameter("lanFolderMountingOptions");

    ret += Glib::ustring::compose ("<folder share=\"%1\" user=\"%2\" password=\"%3\" extra_options=\"%4\"/>\n",
                                   share,
                                   user,
                                   password,
                                   extra_options);

    return ret;
}
void
CoreFunctionUpdate::write_update_xml_file ()
{
    Glib::ustring update_xml;

    update_xml += Glib::ustring::compose(
        "<image type=\"%1\" target=\"%2\">\n",
        _interface == STR_ZIXINF_LANSHAREDFOLDER ? "lanFolder" : "file",
        _target);
    update_xml += "<signature>\n";
    update_xml += _image_param->get_signature();
    update_xml += "</signature>\n";
    update_xml += "<content>";

    // if file, we need to add the path on lanFolder/USB stick
    if (_image_param->get_type() == "file") {
        auto input_update_folder = get_input_update_folder(_filename);

        update_xml += input_update_folder;
        update_xml += "/";
        update_xml += _image_param->get_content();
    } else {
        update_xml += _image_param->get_content();
    }
    update_xml += "</content>\n";

    if (_interface == STR_ZIXINF_LANSHAREDFOLDER) {
        update_xml += get_lan_settings_xml ();
    }
    update_xml += "</image>\n";

    FileHandler::set_file(ZIX_UPDATE_XML_FILE, update_xml);
}

Glib::ustring
CoreFunctionUpdate::get_input_update_folder(const Glib::ustring& full_path) const
{
    if (_filename.empty())
        EXCEPTION("Cannot determine where update came from. " <<
                  "This shouldn't happen.");

    auto dirname = FileHandler::dirname(full_path);

    if (_interface == STR_ZIXINF_USB) {
        PRINT_DEBUG("get_input_update_folder: dirname=" << dirname <<
                    " full_path=" << full_path.raw() <<
                    " usb_mount_path=" << _usb_mount_path.raw());
        return dirname.substr(_usb_mount_path.size(),
                              dirname.size() - _usb_mount_path.size());
    } else {
        PRINT_DEBUG("get_input_update_folder: dirname=" << dirname <<
                    " full_path=" << full_path.raw() <<
                    " lan_mount_path=" << _lan_mount_path.raw());
        return dirname.substr(_lan_mount_path.size(),
                              dirname.size() - _lan_mount_path.size());
    }
}

Glib::ustring
CoreFunctionUpdate::update_image_path() const
{
    if (_filename.empty())
        EXCEPTION("Cannot determine where update came from. " <<
                  "This shouldn't happen.");

    /*
     * For the path we need three things:
     *  - mount path for USB or LAN
     *  - input folder which is relative to the mount path
     *  - file name from the update image itself
     *
     * We already know the path for xml file. So, we just have to append the
     * the update file name to the directory of the original xml file.
     */
    return Glib::build_filename(FileHandler::dirname(_filename),
                                _image_param->get_content());
}

Glib::ustring
CoreFunctionUpdate::ensure_image_in_file (const Glib::ustring & tmpname)
{
    if (_image_param->get_type () == "binary") {
        /* binary image:
         * content is uuencoded file data, write it to tmpname
         */
        auto content_dec = FileHandler::base64_decode (_image_param->get_content ());
        FileHandler::set_file(tmpname, content_dec);
        return tmpname;
    } else if (_image_param->get_type () == "file") {
        return update_image_path();
    } else {
        /* shall never be reached
         */
        return Glib::ustring ("");
    }
}

Glib::ustring
CoreFunctionUpdate::get_encoded_image ()
{
    if (_image_param->get_type () == "binary") {
        /* binary image:
         * content is uuencoded file data, we can just return it.
         */
        return _image_param->get_content ();
    } else if (_image_param->get_type () == "file") {
        auto file = update_image_path();
        auto file_content = FileHandler::get_file(file);

        return FileHandler::base64_encode(file_content);
    } else {
        /* shall never be reached
         */
        return Glib::ustring ("");
    }
}

void CoreFunctionUpdate::handle_sic ()
{
    /* before rebooting, we tell the DC, that we
     * are going to reboot.
     */
    auto dc = QueryClient::get_instance();
    auto xq = XmlQuerySetParameter::create("statusSIC", "restart");
    _query = dc->create_query(xq);
    _query->finished.connect(
        sigc::mem_fun(*this, &CoreFunctionUpdate::on_reboot_query_finish));
    dc->execute(_query);
}

void CoreFunctionUpdate::on_reboot_query_finish(const Glib::RefPtr<XmlResult>& result)
{
    if (result->get_status () != 200) {
        call_finished (result);
        return;
    }

    try {
        std::vector<std::string> script_args;

        // write u-boot file first
        if (_target == "SICBL") {
            /* Bootloader Update does not happen via rescue system,
             * Do it via update_firmware.sh
             *
             * however, this also involves a reboot.
             */
            auto fname = ensure_image_in_file ("/opt/firmware/firmware-bootloader.swu");
            script_args.emplace_back("update_firmware.sh");
            script_args.emplace_back(fname);
        } else {
            /* write update file
             */
            write_update_xml_file();
            script_args.emplace_back("run_rescue.sh");
        }

        // no process timeout
        _update_proc = ProcessRequest::create(script_args);
        _update_proc->finished.connect(
            sigc::mem_fun(*this, &CoreFunctionUpdate::on_update_script_finish));
        _update_proc->start_process();
    } catch (const std::exception& ex) {
        XML_RESULT_INTERNAL_DEVICE_ERROR("Failed to start update script for SIC: " << ex.what());
        return;
    }
}

void CoreFunctionUpdate::handle_dc()
{
    try {
        /* first make sure the data is residing in a file
         */
        _sig_verify_fname = ensure_image_in_file ("/tmp/dcfirmware");

        /* now create request
         */
        _sig_verify_proc = UpdateSignatureCheckRequest::create (_sig_verify_fname, _image_param->get_signature ());
        _sig_verify_proc->finished.connect (
            sigc::mem_fun(*this, &CoreFunctionUpdate::on_dc_signature_verified));
        _sig_verify_proc->start_check ();
    } catch (const std::exception & e) {
        XML_RESULT_INTERNAL_DEVICE_ERROR("Failed to start verify DC update signature: " << e.what());
        return;
    }
}

void CoreFunctionUpdate::on_dc_signature_verified (const Glib::RefPtr <SignatureCheckResult> & result)
{
    /* check, whether we need to cleanup the update
     * file that was written to disk from the xml base64 encoded
     * stuff
     */
    if (_sig_verify_fname == "/tmp/dcfirmware") {
        FileHandler::del_file ("/tmp/dcfirmware");
    }

    /* now check verification result
     */
    if (!result->result ()) {
        /* when the signature validation fails, we return an error
         * from the procedure call, and are finished.
         */
        XML_RESULT_FORBIDDEN (result->error_msg ());
        return;
    }

    /* now that the signature is varified, we can send
     * the update along to the DC
     */
    auto dc = QueryClient::get_instance();
    auto encoded_image = get_encoded_image ();

    if (encoded_image.empty()) {
        XML_RESULT_INTERNAL_DEVICE_ERROR("Unable to obtain image data");
        return;
    }

    auto xq = XmlQuerySetFile::create("binary", _target == "DCBL" ? "BL" : "F", encoded_image);
    _query = dc->create_query(xq);
    _query->finished.connect(
        sigc::mem_fun(*this, &CoreFunctionUpdate::on_query_finish));
    dc->execute(_query);
}

void CoreFunctionUpdate::on_query_finish(const Glib::RefPtr<XmlResult>& result)
{
    int result_code = result->get_status ();

    if (result_code == 500) {
        /* this is a connection error
         * this currently happens, because DC usbserial goes down
         * before we read the result
         *
         * we want to reset the connection now anayways.
         * So... this is not an error
         */
        result_code = 200;
    }

    if (result_code != 200) {
        call_finished(result);
        return;
    }

    PRINT_DEBUG ("CoreFunctionUpdate::on_query_finish (): resetting interface");

    auto dc = QueryClient::get_instance();
    dc->resetted.connect (
        sigc::mem_fun(*this, &CoreFunctionUpdate::on_reset_interface_finish));
    dc->reset_connection ();
}

void CoreFunctionUpdate::on_reset_interface_finish ()
{
    PRINT_DEBUG ("CoreFunctionUpdate::on_reset_interface_finish ():");
    /* interface seems to be there again.
     * wait for X seconds, until DC has completed
     * booting
     */

    auto conf_handler = ConfHandler::get_instance();
    auto parameter    = conf_handler->getParameter("dcRebootDuration");
    int  timeout      = DC_BOOT_TIMEOUT;

    if (!parameter.empty()) {
        timeout = std::atoi(parameter.c_str());
        if (timeout <= 0)
            timeout = DC_BOOT_TIMEOUT;
    }
    Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &CoreFunctionUpdate::on_wait_dc_boot_timeout),
        timeout * 1000);
}

bool CoreFunctionUpdate::on_wait_dc_boot_timeout ()
{
    PRINT_DEBUG ("CoreFunctionUpdate::on_wait_dc_boot_timeout ():");

    auto dc = QueryClient::get_instance();
    auto xq = StateVariable::create_replay_query ();
    _query = dc->create_query(xq);
    _query->finished.connect(
        sigc::mem_fun(*this, &CoreFunctionUpdate::on_query2_finish));
    dc->execute(_query);

    /* we dont want to get called again
     */
    return false;
}

void CoreFunctionUpdate::on_query2_finish(const Glib::RefPtr<XmlResult>& result)
{
    PRINT_DEBUG ("CoreFunctionUpdate::on_query2_finish ():");

    // update network parameters as well
    auto network_config = CNetworkConfig::get_instance();
    network_config->replay_network_settings();

    call_finished(result);
}

void CoreFunctionUpdate::on_update_script_finish(const Glib::RefPtr<ProcessResult>& result)
{
    if (!result->success()) {
        XML_RESULT_INTERNAL_DEVICE_ERROR("Update script for SIC failed: " <<
                                         result->error_reason());
        return;
    }

    if (_target == "SICBL") {
        /* target SICBL does not require us to reboot
         * so right now, the update should be through, and we can reply
         * with success
         */
        XML_RESULT_OK("");
    } else {
        /* all other sic targets require a reboot, which is probably ongoing
         * right now. We just sit and wait now. so we just return
         */
    }
}

void CoreFunctionUpdate::start_call()
{
    std::string dir;

    if (!check_xml_and_get_values()) {
        return;
    }

    try {
        if ( _target == "SICBL" || _target == "SICF" || _target == "SICR" || _target == "SICK" )
            handle_sic();
        else
            handle_dc();
    } catch (const std::exception& ex) {
        XML_RESULT_INTERNAL_DEVICE_ERROR("Update failed: " << ex.what());
    }
}
