#ifndef _XML_QUERY_GET_MEASUREMENTS_LIST_H_
#define _XML_QUERY_GET_MEASUREMENTS_LIST_H_

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

#include "xml_query.h"
#include "xml_file.h"

class XmlQueryGetMeasurementsList : public XmlQuery
{
public:
    XmlQueryGetMeasurementsList(const Glib::ustring& scope);
    ~XmlQueryGetMeasurementsList();

    static Glib::RefPtr<XmlQuery> create(const Glib::ustring& scope);
    Glib::ustring to_xml() const;

private:
    Glib::ustring _scope;
};

#endif /* _XML_QUERY_GET_MEASUREMENTS_LIST_H_ */
