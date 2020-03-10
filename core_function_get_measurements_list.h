#ifndef _CORE_FUNCTION_GET_MEASUREMENTS_LIST_H_
#define _CORE_FUNCTION_GET_MEASUREMENTS_LIST_H_

#include <string>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include "core_function_call.h"
#include "xml_parameter_list.h"
#include "query_client.h"
#include "xml_result.h"
#include "xml_query.h"

/**
 * \brief getMeasurementsList function
 */
class CoreFunctionGetMeasurementsList : public CoreFunctionCall
{
public:
    CoreFunctionGetMeasurementsList(
        XmlParameterList parameters,
        Glib::RefPtr<XmlDescription> description,
        const Glib::ustring& text);

    ~CoreFunctionGetMeasurementsList();

    void start_call();

    static Glib::RefPtr<CoreFunctionCall> factory(XmlParameterList parameters,
						  Glib::RefPtr <XmlDescription> description,
						  const Glib::ustring & text,
						  const xmlpp::Element * en);

private:
    Glib::ustring _target;
    Glib::ustring _scope;
    Glib::RefPtr<Query> _query;

    bool check_xml_and_get_values();
    void handle_sic();
    void handle_dc();

    void on_query_finish(const Glib::RefPtr<XmlResult>& result);
};

#endif /* _CORE_FUNCTION_GET_MEASUREMENTS_LIST_H_ */
