#ifndef _XML_RESTRICTION_PARAMETER_H_
#define _XML_RESTRICTION_PARAMETER_H_

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include <libxml++/nodes/element.h>

#include "xml_parameter.h"
#include "xml_restriction_parameter.h"

/**
 * \brief XmlParameter representing a parameter inside <restriction> argument
 */
class XmlRestrictionParameter : public XmlParameter
{
public:
    XmlRestrictionParameter(const xmlpp::Element *en);
    XmlRestrictionParameter(const Glib::ustring & id);
    ~XmlRestrictionParameter();

    static Glib::RefPtr<XmlRestrictionParameter> create(const xmlpp::Element *en);
    static Glib::RefPtr<XmlRestrictionParameter> create(const Glib::ustring & id);
    static Glib::RefPtr<XmlParameter> param_factory(const xmlpp::Element *en);

    const Glib::ustring& id() const { return _id; }
    Glib::ustring& id() { return _id; }
    const Glib::ustring& exp() const { return _exp; }
    Glib::ustring& exp() { return _exp; }
    const Glib::ustring& nexp() const { return _nexp; }
    Glib::ustring& nexp() { return _nexp; }
    const Glib::ustring& value() const { return _value; }
    Glib::ustring& value() { return _value; }

    void dump() override;

    Glib::ustring to_xml() const override;

    /**
     * This function evaluates the boolean expression given by exp or nexp. But
     * you have to make sure that the _value is set. The actual has to be
     * queried by using getParameter function call on DC. The DC will be reply
     * the value and it can be via value() = "foobar". After that, exp XOR nexp
     * will be evaluated the result value contains true XOR false.
     *
     * \note Throws exception if something is misconfigured e.g. exp and nexp
     * are set or no value exists or if the boolean expression is invalid.
     *
     * @return evaluation of expression
     */
    bool evaluate() const;

private:
    Glib::ustring _id;
    Glib::ustring _exp;
    Glib::ustring _nexp;
    Glib::ustring _value;
};

#endif /* _XML_RESTRICTION_PARAMETER_H_ */
