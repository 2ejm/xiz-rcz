#ifndef _XML_QUERY_GET_FILE_H_
#define _XML_QUERY_GET_FILE_H_

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

#include "xml_query.h"
#include "xml_file.h"

class XmlQueryGetFile : public XmlQuery
{
public:
    XmlQueryGetFile(const Glib::ustring& file);
    ~XmlQueryGetFile();

    static Glib::RefPtr<XmlQuery> create(const Glib::ustring& file);
    Glib::ustring to_xml() const;

private:
    Glib::ustring _file;
};

#endif /* _XML_QUERY_GET_FILE_H_ */
