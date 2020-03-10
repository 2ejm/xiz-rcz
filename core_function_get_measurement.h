#ifndef _CORE_FUNCTION_GET_MEASUREMENT_H_
#define _CORE_FUNCTION_GET_MEASUREMENT_H_

#include <string>
#include <vector>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include "core_function_call.h"
#include "xml_parameter_list.h"
#include "xml_measurement.h"
#include "xml_result.h"
#include "read_file_request.h"
#include "read_file_result.h"
#include "process_request.h"
#include "process_result.h"
#include "post_processing_builder.h"

/**
 * \brief GetMeasurement function
 */
class CoreFunctionGetMeasurement : public CoreFunctionCall
{
public:
    CoreFunctionGetMeasurement(
        XmlParameterList parameters,
        Glib::RefPtr<XmlDescription> description,
	const Glib::ustring & textbody);

    ~CoreFunctionGetMeasurement();

    void start_call();

    static Glib::RefPtr <CoreFunctionCall> factory (XmlParameterList parameters,
                                                    Glib::RefPtr <XmlDescription> description,
                                                    const Glib::ustring & textbody,
						    const xmlpp::Element * en);

private:
    std::list<Glib::RefPtr<XmlMeasurement> > _measurements;
    PostProcessingBuilder::PostProcessingQueue _pp_queue;
    PostProcessingBuilder::PostProcessingQueue::const_iterator _pp_it;
    std::vector<Glib::ustring> _return_values;
    Glib::RefPtr<ReadFileRequest> _read_req;
    Glib::RefPtr<ProcessRequest> _pp_proc;
    Glib::ustring _id;
    Glib::ustring _iid;
    Glib::ustring _format;

    void process_xml_params();
    void build_pp_queue();
    Glib::ustring build_result_xml(const PostProcessingEntry& entry, const std::string& content) const;

    void start_reading();
    void start_post_processing();
    void start_post_proc();

    void on_read_finished(const Glib::RefPtr<ReadFileResult>& result);
    void on_post_proc_finished(const Glib::RefPtr<ProcessResult>& result);
};

#endif /* _CORE_FUNCTION_GET_MEASUREMENT_H_ */
