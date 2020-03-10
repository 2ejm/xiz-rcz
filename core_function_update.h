#ifndef _CORE_FUNCTION_UPDATE_H_
#define _CORE_FUNCTION_UPDATE_H_

#include <string>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include "core_function_call.h"
#include "xml_parameter_list.h"
#include "process_request.h"
#include "process_result.h"
#include "query_client.h"
#include "update_signature_check_request.h"
#include "xml_result.h"
#include "xml_query.h"
#include "xml_image.h"
#include "folder.h"

#define ZIX_UPDATE_XML_FILE "/opt/transfer/update.xml"

/**
 * \brief update function
 */
class CoreFunctionUpdate : public CoreFunctionCall
{
public:
    CoreFunctionUpdate(
        XmlParameterList parameters,
        Glib::RefPtr<XmlDescription> description,
        const Glib::ustring& text);

    ~CoreFunctionUpdate();

    void start_call();

    static Glib::RefPtr<CoreFunctionCall> factory (XmlParameterList parameters,
						   Glib::RefPtr <XmlDescription> description,
						   const Glib::ustring & text,
						   const xmlpp::Element * en);

private:
    Glib::ustring _target;
    Glib::ustring _method;
    Glib::RefPtr <XmlImage> _image_param;

    Glib::ustring _usb_mount_path;
    Glib::ustring _lan_mount_path;

    Glib::RefPtr<Query> _query;
    Glib::RefPtr<ProcessRequest> _update_proc;

    Glib::RefPtr<UpdateSignatureCheckRequest> _sig_verify_proc;
    Glib::ustring _sig_verify_fname;

    bool check_xml_and_get_values();
    void handle_sic();
    void handle_dc();

    Glib::ustring get_lan_settings_xml ();
    void write_update_xml_file ();
    Glib::ustring ensure_image_in_file (const Glib::ustring & tmpname);
    Glib::ustring get_encoded_image ();
    Glib::ustring update_image_path() const;
    Glib::ustring get_input_update_folder(const Glib::ustring& full_path) const;

    void on_dc_signature_verified (const Glib::RefPtr <SignatureCheckResult> & result);
    void on_query_finish(const Glib::RefPtr<XmlResult>& result);
    void on_reboot_query_finish(const Glib::RefPtr<XmlResult>& result);
    void on_reset_interface_finish();
    bool on_wait_dc_boot_timeout ();
    void on_query2_finish(const Glib::RefPtr<XmlResult>& result);
    void on_update_script_finish(const Glib::RefPtr<ProcessResult>& result);
};

#endif /* _CORE_FUNCTION_UPDATE_H_ */
