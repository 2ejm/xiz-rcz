#ifndef _CORE_FUNCTION_DATA_IN_H_
#define _CORE_FUNCTION_DATA_IN_H_

#include <string>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include "core_function_call.h"
#include "xml_parameter_list.h"
#include "process_request.h"
#include "process_result.h"

/**
 * \brief dataIn function
 */
class CoreFunctionDataIn : public CoreFunctionCall
{
public:
    CoreFunctionDataIn(
        XmlParameterList parameters,
        Glib::RefPtr<XmlDescription> description,
        const Glib::ustring& text);

    ~CoreFunctionDataIn();

    void start_call();

    static Glib::RefPtr<CoreFunctionCall> factory (XmlParameterList parameters,
						   Glib::RefPtr <XmlDescription> description,
						   const Glib::ustring & text,
						   const xmlpp::Element * en);

private:
    Glib::ustring _src;
    Glib::ustring _type;
    Glib::RefPtr<ProcessRequest> _ip_proc;

    bool check_xml_and_get_values();
    std::string get_input_processor() const;
    void start_ip();
    void on_ip_proc_finish(const Glib::RefPtr<ProcessResult>& result);
};

#endif /* _CORE_FUNCTION_DATA_IN_H_ */
