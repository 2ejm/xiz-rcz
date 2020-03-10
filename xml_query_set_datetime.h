#ifndef _XML_QUERY_SET_DATETIME_H_
#define _XML_QUERY_SET_DATETIME_H_

#include <glibmm/refptr.h>

#include <vector>

#include "xml_query.h"
#include "xml_function.h"

class XmlQuerySetDateTime : public XmlQuery
{
public:
    explicit
    XmlQuerySetDateTime(const Glib::ustring & date, const Glib::ustring & time)
	: XmlQuery ("setParameter")
	, _date(date)
	, _time (time)
    {}

    virtual ~XmlQuerySetDateTime()
    {}

    static Glib::RefPtr<XmlQuery>
    create(const Glib::ustring & date, const Glib::ustring & time)
    {
        return Glib::RefPtr<XmlQuery>(new XmlQuerySetDateTime (date, time));
    }

    Glib::ustring to_xml() const;

private:
    Glib::ustring _date;
    Glib::ustring _time;
};

#endif /* _XML_QUERY_SET_DATETIME_H_ */
