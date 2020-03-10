#ifndef _XML_QUERY_REPLAY_STATUS_H_
#define _XML_QUERY_REPLAY_STATUS_H_

#include <glibmm/refptr.h>

#include <vector>

#include "xml_query.h"
#include "xml_function.h"

class XmlQueryReplayStatus : public XmlQuery
{
public:
    explicit
    XmlQueryReplayStatus(const std::vector <Glib::ustring> & ids, const std::vector <Glib::ustring> & values)
	: XmlQuery ("setParameter")
	, _ids(ids)
	, _values (values)
    {}

    virtual ~XmlQueryReplayStatus()
    {}

    static Glib::RefPtr<XmlQuery>
    create(const std::vector <Glib::ustring> & ids, const std::vector <Glib::ustring> & values)
    {
        return Glib::RefPtr<XmlQuery>(new XmlQueryReplayStatus (ids, values));
    }

    Glib::ustring to_xml() const;

private:
    std::vector <Glib::ustring> _ids;
    std::vector <Glib::ustring> _values;
};

#endif /* _XML_QUERY_REPLAY_STATUS_H_ */
