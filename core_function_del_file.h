#ifndef _CORE_FUNCTION_DEL_FILE_H_
#define _CORE_FUNCTION_DEL_FILE_H_

#include <string>
#include <list>

#include <glibmm/refptr.h>

#include "core_function_call.h"
#include "xml_parameter_list.h"
#include "query_client.h"
#include "xml_result.h"
#include "xml_query.h"
#include "xml_file.h"

/**
 * \brief DelFile function
 *
 * This function is called via
 *  <function fid="delFile" id="Zeiss.bmp" /> or
 *  <function fid="delFile">
 *    <file id="Zeiss.bmp" />
 *    <file id="veeman.bmp" />
 *  </function>
 *
 * Parameters: Host (DC || SIC) ; File(s) ; Src ; Id
 *
 */
class CoreFunctionDelFile : public CoreFunctionCall
{
public:
    CoreFunctionDelFile(
        XmlParameterList parameters,
        Glib::RefPtr<XmlDescription> description,
	const Glib::ustring & textbody);

    ~CoreFunctionDelFile();

    void start_call();

    static Glib::RefPtr <CoreFunctionCall> factory (XmlParameterList parameters,
                                                    Glib::RefPtr <XmlDescription> description,
                                                    const Glib::ustring & textbody,
						    const xmlpp::Element * en);

private:
    std::list<Glib::RefPtr<XmlFile> > _files;
    std::list<Glib::RefPtr<XmlFile> >::const_iterator _del_it;
    Glib::RefPtr<Query> _write_query;

    void check_file_list() const;
    void handle_one_file(const Glib::RefPtr<XmlFile>& file);
    void on_write_query_finish(const Glib::RefPtr<XmlResult>& result);
};

#endif /* _CORE_FUNCTION_DEL_FILE_H_ */
