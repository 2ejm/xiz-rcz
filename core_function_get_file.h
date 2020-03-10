#ifndef _CORE_FUNCTION_GET_FILE_H_
#define _CORE_FUNCTION_GET_FILE_H_

#include <string>
#include <list>
#include <vector>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include "core_function_call.h"
#include "xml_parameter_list.h"
#include "read_file_request.h"
#include "read_file_result.h"
#include "query_client.h"
#include "xml_file.h"

/**
 * \brief GetFile function
 *
 * This function is called via
 *  <function fid="getFile" id="Zeiss.bmp" /> or
 *  <function fid="getFile">
 *    <file id="Zeiss.bmp" />
 *    <file id="veeman.bmp" />
 *  </function>
 *
 * Parameters: Host (DC || SIC) ; File(s) ; Id
 *
 */
class CoreFunctionGetFile : public CoreFunctionCall
{
public:
    CoreFunctionGetFile(
        XmlParameterList parameters,
        Glib::RefPtr<XmlDescription> description,
	const Glib::ustring & textbody);

    ~CoreFunctionGetFile();

    void start_call();

    static Glib::RefPtr <CoreFunctionCall> factory (XmlParameterList parameters,
						    Glib::RefPtr <XmlDescription> description,
						    const Glib::ustring & textbody,
						    const xmlpp::Element * en);

private:
    Glib::RefPtr<ReadFileRequest> _read_proc;
    Glib::RefPtr<Query> _write_query;
    std::list<Glib::RefPtr<XmlFile> > _files;
    std::list<Glib::RefPtr<XmlFile> >::const_iterator _proc_read_it;
    std::vector<Glib::ustring> _return_values;

    void addDirectory( Glib::ustring directoryName);
    void check_file_list() const;

    void on_read_finish(const Glib::RefPtr<ReadFileResult>& result);
    void on_write_query_finish(const Glib::RefPtr<XmlResult>& result);
    void handle_one_file(const Glib::RefPtr<XmlFile>& file);
};

#endif /* _CORE_FUNCTION_GET_FILE_H_ */
