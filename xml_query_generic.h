
#ifndef ZIX_XML_QUERY_GENERIC_H
#define ZIX_XML_QUERY_GENERIC_H

#include "xml_query.h"
#include "xml_function.h"

#include <glibmm/refptr.h>

class XmlQueryGeneric : public XmlQuery
{
    public:
	XmlQueryGeneric (Glib::ustring fid, XmlParameterList parameters, Glib::ustring textbody);
	virtual ~XmlQueryGeneric ();

	static Glib::RefPtr <XmlQueryGeneric> create (Glib::ustring fid,
						      XmlParameterList parameters,
						      Glib::ustring textbody);

	virtual Glib::ustring to_xml () const;

    private:
	XmlParameterList _parameters;
	//Glib::RefPtr<XmlDescription> _description;
	//std::list <XmlRestriction> _restrictions;
	//Glib::ustring _dest;
	Glib::ustring _textbody;
};

#endif
