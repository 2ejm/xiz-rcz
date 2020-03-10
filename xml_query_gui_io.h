#ifndef ZIX_XML_QUERY_GUI_IO_H
#define ZIX_XML_QUERY_GUI_IO_H

#include "xml_query.h"
#include "xml_description.h"

class XmlQueryGuiIO : public XmlQuery
{
    public:
	XmlQueryGuiIO (Glib::RefPtr <XmlDescription> proc_desc,
		       Glib::RefPtr <XmlDescription> step_desc);

	XmlQueryGuiIO (Glib::RefPtr <XmlDescription> step_desc, int lid);
	XmlQueryGuiIO ();

	~XmlQueryGuiIO ();

	static Glib::RefPtr <XmlQuery> create (Glib::RefPtr <XmlDescription> proc_desc,
					       Glib::RefPtr <XmlDescription> step_desc);

	static Glib::RefPtr <XmlQuery> create (Glib::RefPtr <XmlDescription> step_desc, int lid);
	static Glib::RefPtr <XmlQuery> create ();

	Glib::ustring to_xml () const;

    private:
	Glib::ustring to_xml_proc_start () const;
	Glib::ustring to_xml_proc_step  () const;
	Glib::ustring to_xml_proc_finish  () const;

	Glib::RefPtr <XmlDescription> _proc_desc;
	Glib::RefPtr <XmlDescription> _step_desc;

	int _lid;

	enum { PROC_START, PROC_STEP, PROC_FINISH } _mode;
};

#endif
