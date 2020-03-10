#ifndef _XML_QUERY_DEL_FILE_H_
#define _XML_QUERY_DEL_FILE_H_

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

#include "xml_query.h"
#include "xml_file.h"

class XmlQueryDelFile : public XmlQuery
{
public:
    XmlQueryDelFile(const Glib::ustring& file);
    ~XmlQueryDelFile();

    static Glib::RefPtr<XmlQuery> create(const Glib::ustring& file);
    Glib::ustring to_xml() const;

private:
    Glib::ustring _file;
};

#endif /* _XML_QUERY_DEL_FILE_H_ */
