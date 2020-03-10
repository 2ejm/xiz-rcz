#ifndef ZIX_XML_QUERY_PASS_H
#define ZIX_XML_QUERY_PASS_H
//-----------------------------------------------------------------------------
///
/// \brief  XML Query that passes xml request to through target
///
///         Usually XML Queries interprets xml structure and reassembles it on
///         sending to ither target.
///         This class reads the original xml tree, removes some nodes
///         and returns (to_xml()) the string. The origin XML structure is kept.
///
//-----------------------------------------------------------------------------


#include "xml_query.h"
#include "xml_function.h"

#include <glibmm/refptr.h>


class XmlQueryPass : public XmlQuery
{
    private:
        Glib::ustring _xml;

    public:
    XmlQueryPass (Glib::ustring fid, Glib::ustring xml);
    virtual ~XmlQueryPass ();

    static Glib::RefPtr <XmlQueryPass> create (Glib::ustring fid, Glib::ustring xml);

	virtual Glib::ustring to_xml () const;

};

#endif
