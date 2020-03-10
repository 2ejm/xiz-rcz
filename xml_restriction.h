#ifndef _XML_RESTRICTION_H_
#define _XML_RESTRICTION_H_

#include <vector>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include <libxml++/nodes/element.h>

#include "xml_parameter.h"
#include "xml_restriction_parameter.h"

/**
 * \brief XmlParameter representing <restriction> argument
 *
 * This class holds all XmlRestrictionParameters, that are found inside one
 * restriction.
 */
class XmlRestriction : public XmlParameter
{

public:
    using RestrictionParameterList = std::vector<Glib::RefPtr<XmlRestrictionParameter> >;

    XmlRestriction(const xmlpp::Element *en);
    ~XmlRestriction();

    static Glib::RefPtr<XmlRestriction> create(const xmlpp::Element *en);
    static Glib::RefPtr<XmlParameter> param_factory(const xmlpp::Element *en);

    const RestrictionParameterList& params() const
    {
        return _params;
    }

    RestrictionParameterList& params()
    {
        return _params;
    }

    void dump() override;

    Glib::ustring to_xml() const override;

private:
    RestrictionParameterList _params;
};

#endif /* _XML_RESTRICTION_H_ */
