#ifndef _CORE_FUNCTION_GET_FILES_LIST_H_
#define _CORE_FUNCTION_GET_FILES_LIST_H_

#include <glibmm/refptr.h>

#include "core_function_call.h"
#include "xml_parameter_list.h"
#include "query_client.h"
#include "xml_result.h"
#include "xml_query.h"

/**
 * \brief GetFilesList function
 *
 * This function is called via
 *  <function fid="getFilesList" /> or
 *  <function fid="getFilesList" host="SIC" dir="/usr/ZIX/bin" />
 *
 * Parameters: Host (DC || SIC) ; Dir
 *
 */
class CoreFunctionGetFilesList : public CoreFunctionCall
{
public:
    CoreFunctionGetFilesList(
        XmlParameterList parameters,
        Glib::RefPtr<XmlDescription> description,
	const Glib::ustring & textbody);

    ~CoreFunctionGetFilesList();

    void start_call();

    static Glib::RefPtr <CoreFunctionCall> factory (XmlParameterList parameters,
						    Glib::RefPtr <XmlDescription> description,
						    const Glib::ustring & textbody,
						    const xmlpp::Element * en);

private:
    Glib::RefPtr<Query> _write_query;

    std::vector<Glib::ustring> build_reply_xml(const std::string& dir, const std::string& host,
                                               const std::vector<std::string>& dirs) const;
    void on_write_query_finish(const Glib::RefPtr<XmlResult>& result);
};

#endif /* _CORE_FUNCTION_GET_FILES_LIST_H_ */
