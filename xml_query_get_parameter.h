#ifndef _XML_QUERY_GET_PARAMETER_H_
#define _XML_QUERY_GET_PARAMETER_H_

#include <glibmm/refptr.h>

#include <vector>

#include "xml_query.h"
#include "xml_function.h"
#include "xml_restriction.h"

class XmlQueryGetParameter : public XmlQuery
{
public:
    explicit XmlQueryGetParameter(const std::vector<Glib::RefPtr<XmlRestriction> >& restrictions);
    explicit XmlQueryGetParameter (const std::vector <Glib::RefPtr <XmlRestrictionParameter> > & params);
    virtual ~XmlQueryGetParameter();

    static Glib::RefPtr<XmlQuery> create(const std::vector<Glib::RefPtr<XmlRestriction> >& restrictions);
    static Glib::RefPtr <XmlQuery> create (const std::vector <Glib::RefPtr <XmlRestrictionParameter> > & params);

    Glib::ustring to_xml() const;

private:
    std::vector<Glib::RefPtr<XmlRestrictionParameter> > _res_params;
};

#endif /* _XML_QUERY_GET_PARAMETER_H_ */
