#ifndef _XML_SIGNATURE_H_
#define _XML_SIGNATURE_H_

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include <libxml++/nodes/element.h>

#include "xml_parameter.h"

class XmlSignature : public XmlParameter
{
public:
    XmlSignature(const xmlpp::Element *en);
    ~XmlSignature();

    static Glib::RefPtr<XmlSignature> create(const xmlpp::Element *en);
    static Glib::RefPtr<XmlParameter> param_factory(const xmlpp::Element *en);

    const Glib::ustring& signature() const
    {
        return _signature;
    }

    Glib::ustring& signature()
    {
        return _signature;
    }

    void dump() override;

    Glib::ustring to_xml() const override;

private:
    Glib::ustring _signature;
};

#endif /* _XML_SIGNATURE_H_ */
