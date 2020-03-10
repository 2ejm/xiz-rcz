#ifndef _CORE_FUNCTION_DATA_OUT_H_
#define _CORE_FUNCTION_DATA_OUT_H_

#include <string>
#include <vector>
#include <map>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include "core_function_call.h"
#include "xml_parameter_list.h"
#include "process_request.h"
#include "process_result.h"
#include "copy_file_request.h"
#include "copy_file_result.h"
#include "post_processing_builder.h"
#include "file_destination.h"
#include "copy_queue_entry.h"
#include "signature_creation_request.h"
#include "signature_creation_result.h"

/**
 * \brief dataOut function
 */
class CoreFunctionDataOut : public CoreFunctionCall
{
public:
    using CopyQueue = std::vector<CopyQueueEntry>;

    CoreFunctionDataOut(
        XmlParameterList parameters,
        Glib::RefPtr<XmlDescription> description,
        const Glib::ustring & text,
        const xmlpp::Element *_en);

    ~CoreFunctionDataOut();

    void start_call();

    static Glib::RefPtr<CoreFunctionCall> factory (XmlParameterList parameters,
						   Glib::RefPtr <XmlDescription> description,
						   const Glib::ustring & text,
						   const xmlpp::Element * en);

private:
    Glib::RefPtr<ProcessRequest> _post_proc;
    Glib::RefPtr<ProcessRequest> _lp_proc;
    Glib::RefPtr<ProcessRequest> _copy_proc;
    Glib::RefPtr<ProcessRequest> _tar_proc;
    Glib::RefPtr<CopyFileRequest> _copy_req;
    Glib::RefPtr<SignatureCreationRequest> _sig_req;
    Glib::ustring _type;
    Glib::ustring _id;
    Glib::ustring _iid;
    Glib::ustring _format;
    Glib::ustring _output;
    Glib::ustring _protocol;
    Glib::ustring _scheme;
    FileDestination _dest;
    PostProcessingBuilder::PostProcessingQueue _pp_queue;
    CopyQueue _copy_queue;
    std::size_t _post_proc_idx;
    std::size_t _copy_req_idx;
    std::string _serial_number;
    std::string _xml_file;
    const xmlpp::Element *_en;

    void process_xml_params();
    bool check_valid_xml() const;
    void create_post_processing_queue();
    void create_copy_queue();
    bool is_post_processing_needed(const std::string& file) const;
    std::vector<std::string> get_copy_script() const;
    std::string get_serial_number();
    std::string create_config_xml();
    void type_dispatcher();

    void start_postprocessing();
    void start_copying();
    void start_printer_copying();
    void start_file_copying();
    void start_socket_copying();
    void start_tar_process();
    void return_local_printer_result ();
    void start_signature_creation(const std::string& xml_file);

    void on_post_proc_finish(const Glib::RefPtr<ProcessResult>& result);
    void on_lp_proc_finish(const Glib::RefPtr<ProcessResult>& result);
    void on_copy_proc_finish(const Glib::RefPtr<ProcessResult>& result);
    void on_tar_proc_finish(const Glib::RefPtr<ProcessResult>& result);
    void on_copy_finish(const Glib::RefPtr<CopyFileResult>& result);
    void on_signature_creation_finish(const Glib::RefPtr<SignatureCreationResult>& result);
};

#endif /* _CORE_FUNCTION_DATA_OUT_H_ */
