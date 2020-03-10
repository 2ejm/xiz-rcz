
#ifndef ZIX_XML_FUNCTION_H
#define ZIX_XML_FUNCTION_H

#include <map>
#include <list>
#include <vector>

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

#include <libxml++/nodes/element.h>

#include "xml_parameter.h"
#include "xml_parameter_list.h"
#include "xml_description.h"
#include "xml_restriction.h"
#include "xml_signature.h"

#include "function_call.h"

/**
 * \bruief represents a function in xml
 *
 * An XmlFunction might also be a Parameter to
 * a procedure function.
 *
 * This class also provides the facilities to create
 * the CoreFunctionCall object, that represents
 * the function Call closure.
 */
class XmlFunction : public XmlParameter
{
public:
    using XmlRestrictionList = std::vector<Glib::RefPtr<XmlRestriction> >;
    using PrioMap            = std::map<Glib::ustring, int>;

    XmlFunction (const xmlpp::Element *en, const Glib::ustring & interface,
                 const Glib::ustring & filename);

    static Glib::RefPtr <XmlFunction> create (const xmlpp::Element *en,
                                              const Glib::ustring & interface,
                                              const Glib::ustring & filename);

    void dump ();

    /**
     * Create the CoreFunctionCall from this XmlFunction
     *
     * src Arguments are evaluated during this call.
     * so the CoreFunctionCall will see the final
     * parameters.
     *
     * This makes it important, that create_call() is called,
     * after potential src Files have been created.
     */
    Glib::RefPtr <FunctionCall> create_call(bool sig_check);

    const Glib::ustring & get_fid () const { return _fid; }
    const Glib::ustring & get_dest () const { return _dest; }
    const Glib::ustring & get_body () const { return _textbody; }

    const Glib::RefPtr<XmlDescription> get_description () const {
        return _description;
    }

    const XmlParameterList get_params () const { return _parameters; }

    const XmlRestrictionList& restrictions() const
    {
        return _restrictions;
    }

    XmlRestrictionList& restrictions()
    {
        return _restrictions;
    }

    Glib::RefPtr<XmlSignature> signature() const
    {
        return _signature;
    }

    Glib::ustring to_xml () const;

    int prio() const noexcept
    {
        auto it = prio_map.find(_fid);
        if (it == prio_map.end())
            return 0;
        return it->second;
    }

private:
    static const PrioMap prio_map;

    xmlpp::DomParser _file_parser;
    const xmlpp::Element * _elem;
    Glib::ustring _fid;
    XmlParameterList _parameters;
    Glib::RefPtr<XmlDescription> _description;
    XmlRestrictionList _restrictions;
    Glib::RefPtr<XmlSignature> _signature;
    Glib::ustring _dest;
    Glib::ustring _textbody;

    Glib::ustring _src;

    Glib::ustring _interface;

    Glib::ustring _filename;

    void parse_description (xmlpp::Element *en);
    void parse_arg (xmlpp::Element *en);

    Glib::RefPtr <XmlFunction> load_src ();
};

#endif
