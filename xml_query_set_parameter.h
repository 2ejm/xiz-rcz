#ifndef _XML_QUERY_SET_PARAMETER_H_
#define _XML_QUERY_SET_PARAMETER_H_

#include <glibmm/refptr.h>

#include <vector>

#include "xml_query.h"
#include "xml_function.h"

class XmlQuerySetParameter : public XmlQuery
{
public:
    explicit
    XmlQuerySetParameter(const Glib::ustring & id, const Glib::ustring & value)
	: XmlQuery ("setParameter")
	, _id(id)
	, _value (value)
    {}

    virtual ~XmlQuerySetParameter()
    {}

    static Glib::RefPtr<XmlQuery>
    create(const Glib::ustring & id, const Glib::ustring & value)
    {
        return Glib::RefPtr<XmlQuery>(new XmlQuerySetParameter (id, value));
    }

    Glib::ustring to_xml() const;

private:
    Glib::ustring _id;
    Glib::ustring _value;
};

#endif /* _XML_QUERY_SET_PARAMETER_H_ */
