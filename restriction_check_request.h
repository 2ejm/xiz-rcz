#ifndef _RESTRICTION_CHECK_REQUEST_H_
#define _RESTRICTION_CHECK_REQUEST_H_

#include <glibmm/refptr.h>
#include <glibmm/object.h>

#include <sigc++/signal.h>

#include <string>
#include <map>

#include "xml_function.h"
#include "xml_result.h"
#include "xml_query.h"
#include "query_client.h"
#include "restriction_check_result.h"
#include "zix_interface.h"

class RestrictionCheckRequest : public Glib::Object
{
public:
    using RefPtr       = Glib::RefPtr<RestrictionCheckRequest>;
    using RestCheckMap = std::map<std::string, std::map<std::string, bool> >;

    explicit RestrictionCheckRequest(const XmlFunction::XmlRestrictionList & restrictions, const Glib::ustring & fid) :
        _restrictions(restrictions),
	_fid(fid)
    {}

    static inline RefPtr create(const XmlFunction::XmlRestrictionList & restrictions, const Glib::ustring & fid)
    {
        return RefPtr(new RestrictionCheckRequest(restrictions, fid));
    }

    void start_check(const ZixInterface& channel);

    sigc::signal<void, const Glib::RefPtr<RestrictionCheckResult>& > finished;

private:
    static const RestCheckMap rest_check_map;

    XmlFunction::XmlRestrictionList _restrictions;
    Glib::ustring _fid;
    Glib::RefPtr<Query> _write_query;

    bool check_needed(const ZixInterface& channel) const;
    void start_query_dc();
    bool evaluate() const;
    void enrich_values(const Glib::ustring& xml);

    void on_write_query_finish(const Glib::RefPtr<XmlResult>& result);
};

#endif /* _RESTRICTION_CHECK_REQUEST_H_ */
