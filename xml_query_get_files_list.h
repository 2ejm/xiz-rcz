#ifndef _XML_QUERY_GET_FILES_LIST_H_
#define _XML_QUERY_GET_FILES_LIST_H_

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

#include "xml_query.h"
#include "xml_file.h"

class XmlQueryGetFilesList : public XmlQuery
{
public:
    XmlQueryGetFilesList();
    ~XmlQueryGetFilesList();

    static Glib::RefPtr<XmlQuery> create();
    Glib::ustring to_xml() const;
};

#endif /* _XML_QUERY_GET_FILES_LIST_H_ */
