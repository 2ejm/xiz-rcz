#ifndef _CORE_FUNCTION_SET_FILE_H_
#define _CORE_FUNCTION_SET_FILE_H_

#include <string>
#include <list>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include "core_function_call.h"
#include "xml_parameter_list.h"
#include "xml_file.h"
#include "write_file_request.h"
#include "write_file_result.h"
#include "query_client.h"

/**
 * \brief SetFile function
 */
class CoreFunctionSetFile : public CoreFunctionCall
{
public:
    CoreFunctionSetFile(
        XmlParameterList parameters,
        Glib::RefPtr<XmlDescription> description,
	const Glib::ustring & textbody);

    ~CoreFunctionSetFile();

    void start_call();

    static Glib::RefPtr <CoreFunctionCall> factory (XmlParameterList parameters,
						    Glib::RefPtr <XmlDescription> description,
						    const Glib::ustring & textbody,
						    const xmlpp::Element * en);

private:
    Glib::RefPtr<WriteFileRequest> _write_proc;
    std::list<Glib::RefPtr<XmlFile> > _files;
    std::list<Glib::RefPtr<XmlFile> >::const_iterator _proc_write_it;
    Glib::RefPtr<Query> _write_query;

    void check_file_list() const;

    void on_write_finish(const Glib::RefPtr<WriteFileResult>& result);
    void on_write_query_finish(const Glib::RefPtr<XmlResult>& result);
    void handle_one_file(Glib::RefPtr<XmlFile> file);
};

#endif /* _CORE_FUNCTION_SET_FILE_H_ */
