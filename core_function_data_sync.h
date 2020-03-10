#ifndef _CORE_FUNCTION_DATA_SYNC_H_
#define _CORE_FUNCTION_DATA_SYNC_H_

#include <string>
#include <utility>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include "core_function_call.h"
#include "xml_parameter_list.h"
#include "process_request.h"
#include "process_result.h"
#include "write_file_request.h"
#include "write_file_result.h"
#include "post_processor.h"
#include "format.h"

/**
 * \brief dataSync function
 */
class CoreFunctionDataSync : public CoreFunctionCall
{
public:
    using StringPair = std::pair<std::string, std::string>;

    CoreFunctionDataSync(
        XmlParameterList parameters,
        Glib::RefPtr<XmlDescription> description,
        const Glib::ustring& text);

    ~CoreFunctionDataSync();

    void start_call();

    static Glib::RefPtr<CoreFunctionCall> factory (XmlParameterList parameters,
						   Glib::RefPtr <XmlDescription> description,
						   const Glib::ustring & text,
						   const xmlpp::Element * en);

private:
    Glib::ustring _text;
    Glib::ustring _iid;
    Glib::ustring _tag;
    Glib::ustring _type;
    Glib::ustring _dap;
    Glib::ustring _car;
    Glib::ustring _id;
    Glib::ustring _date;
    Glib::ustring _time;
    Glib::ustring _tz;
    Glib::RefPtr<WriteFileRequest> _write_req;
    Glib::RefPtr<ProcessRequest> _proc_req;
    std::string _generated_file;
    std::vector<Glib::ustring> _ret_value;

    bool check_xml_and_get_values();
    std::string build_file_name() const;
    std::string build_pp_file_name(const Format& format) const;
    std::string setup_measurement_dir() const;
    std::string get_database_script() const;
    PostProcessor get_postprocessor(const std::string& format) const;
    StringPair split_car_attribute() const;
    void add_id_mapping(const Glib::ustring& id = "") const;
    StringPair get_and_strip_id(const std::string& stdout) const;
    void delete_post_processing_results() const;

    void start_db_proc();
    void start_car_proc();

    void handle_img(const std::string& file_name);
    void handle_xml(const std::string& file_name);

    void on_write_finish(const Glib::RefPtr<WriteFileResult>& result);
    void on_database_finish(const Glib::RefPtr<ProcessResult>& result);
    void on_car_finish(const Glib::RefPtr<ProcessResult>& result);
};

#endif /* _CORE_FUNCTION_DATA_SYNC_H_ */
