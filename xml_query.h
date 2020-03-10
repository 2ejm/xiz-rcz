
#ifndef ZIX_XML_QUERY_H
#define ZIX_XML_QUERY_H

#include "glibmm/ustring.h"
#include "glibmm/object.h"

class XmlQuery : public Glib::Object
{
    protected:
	XmlQuery (const Glib::ustring & fid);
	virtual ~XmlQuery ();

	Glib::ustring _fid;

    public:
	virtual Glib::ustring to_xml () const = 0;
};
#endif
