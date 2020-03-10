
#include "xml_query_gui_io.h"

XmlQueryGuiIO::XmlQueryGuiIO (Glib::RefPtr <XmlDescription> proc_desc,
			      Glib::RefPtr <XmlDescription> step_desc)
    : XmlQuery ("guiIO")
    , _proc_desc (proc_desc)
    , _step_desc (step_desc)
    , _mode (PROC_START)
{ }


XmlQueryGuiIO::XmlQueryGuiIO (Glib::RefPtr <XmlDescription> step_desc, int lid)
    : XmlQuery ("guiIO")
    , _step_desc (step_desc)
    , _lid (lid)
    , _mode (PROC_STEP)
{ }

XmlQueryGuiIO::XmlQueryGuiIO ()
    : XmlQuery ("guiIO")
    , _mode (PROC_FINISH)
{ }

Glib::RefPtr <XmlQuery>
XmlQueryGuiIO::create (Glib::RefPtr <XmlDescription> proc_desc,
		       Glib::RefPtr <XmlDescription> step_desc)
{
    return Glib::RefPtr <XmlQuery> (new XmlQueryGuiIO (proc_desc, step_desc));
}

Glib::RefPtr <XmlQuery>
XmlQueryGuiIO::create (Glib::RefPtr <XmlDescription> step_desc, int lid)
{
    return Glib::RefPtr <XmlQuery> (new XmlQueryGuiIO (step_desc, lid));
}

Glib::RefPtr <XmlQuery>
XmlQueryGuiIO::create ()
{
    return Glib::RefPtr <XmlQuery> (new XmlQueryGuiIO ());
}

XmlQueryGuiIO::~XmlQueryGuiIO ()
{ }


Glib::ustring
XmlQueryGuiIO::to_xml_proc_start () const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose ("<function fid=\"%1\" type=\"message\" mode=\"init\" hsize=\"470\" vsize=\"430\">\n", _fid);
    ret += "\t<line fontsize=\"large\">\n";
    if (_proc_desc)
	ret += _proc_desc->to_body ();
    else {
	ret += "<language name=\"English\" default=\"1\">No Text</language>\n";
    }
    ret += "\t</line>\n";
    ret += "\t<line lid=\"1\">\n";
    if (_step_desc)
	ret += _step_desc->to_body ();
    else {
	ret += "<language name=\"English\" default=\"1\">No Text</language>\n";
    }
    ret += "\t</line>\n";
    ret += "</function>\n";

    return ret;
}

Glib::ustring
XmlQueryGuiIO::to_xml_proc_step () const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose ("<function fid=\"%1\" mode=\"update\">\n", _fid);
    ret += Glib::ustring::compose ("\t<line lid=\"%1\" halign=\"right\" mode=\"append\">\n", _lid - 1);
    ret += "\t\t<language name=\"English\" default=\"1\">Done</language>\n";
    ret += "\t\t<language name=\"Deutsch\">Fertig</language>\n";
    ret += "\t</line>\n";
    ret += Glib::ustring::compose ("\t<line lid=\"%1\">\n", _lid);
    if (_step_desc)
	ret += _step_desc->to_body ();
    else {
	ret += "<language name=\"English\" default=\"1\">No Text</language>\n";
    }
    ret += "\t</line>\n";
    ret += "</function>\n";

    return ret;
}

Glib::ustring
XmlQueryGuiIO::to_xml_proc_finish () const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose ("<function fid=\"%1\" mode=\"close\"/>\n", _fid);

    return ret;
}

Glib::ustring
XmlQueryGuiIO::to_xml () const
{
    Glib::ustring ret;

    switch (_mode) {
	case PROC_STEP:
	    return to_xml_proc_step ();
	case PROC_START:
	    return to_xml_proc_start ();
	case PROC_FINISH:
	    return to_xml_proc_finish ();
    }

    return Glib::ustring ();
}
