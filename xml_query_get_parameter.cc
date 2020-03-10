#include <map>

#include "xml_query_get_parameter.h"

XmlQueryGetParameter::XmlQueryGetParameter(const std::vector<Glib::RefPtr<XmlRestriction> >& restrictions)
    : XmlQuery ("getParameter")
{
    /* XmlQueryGetParameter is only interested in the list of restriction parameters.
     *
     * In this constructor we extract all params from the passed restrictions,
     * and accumulate them in _res_params.
     */
    std::map<Glib::ustring, bool> id_map;

    for (auto&& res : restrictions) {
        for (auto&& par : res->params()) {
            auto it = id_map.find(par->id());
            if (it != id_map.end())
                continue;
            id_map[par->id()] = true;
	    _res_params.push_back (par);
        }
    }
}

XmlQueryGetParameter::XmlQueryGetParameter (const std::vector <Glib::RefPtr <XmlRestrictionParameter> > & params)
    : XmlQuery ("getParameter")
    , _res_params (params)
{ }

XmlQueryGetParameter::~XmlQueryGetParameter()
{ }

Glib::RefPtr <XmlQuery>
XmlQueryGetParameter::create (const std::vector <Glib::RefPtr <XmlRestriction> > & restrictions)
{
    return Glib::RefPtr <XmlQuery> (new XmlQueryGetParameter (restrictions));
}

Glib::RefPtr <XmlQuery>
XmlQueryGetParameter::create (const std::vector <Glib::RefPtr <XmlRestrictionParameter> > & params)
{
    return Glib::RefPtr <XmlQuery> (new XmlQueryGetParameter (params));
}

Glib::ustring XmlQueryGetParameter::to_xml() const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose("<function fid=\"%1\">\n", _fid);
    for (auto&& par : _res_params) {
	ret += "  " + par->to_xml();
    }
    ret += "</function>\n";

    return ret;
}
