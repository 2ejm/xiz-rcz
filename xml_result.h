
#ifndef ZIX_XML_RESULT_H
#define ZIX_XML_RESULT_H

#include <vector>

#include "glibmm/object.h"

#include "xml_parameter_list.h"

/**
 * \brief Baseclass for Result of CoreFunctionCall
 *
 * The XmlResult can be converted to a Glib::ustring of
 * xml, which is then emitted via the InterfaceHandler
 * emit_result function
 */
class XmlResult : public Glib::Object
{
public:
    XmlResult();
    XmlResult(int status);
    XmlResult(int status, const Glib::ustring & message);
    XmlResult(int status, const Glib::ustring & message,
              const std::vector<Glib::ustring>& return_values);

    static Glib::RefPtr <XmlResult> create (int status);
    static Glib::RefPtr <XmlResult> create (int status, const Glib::ustring & message);
    static Glib::RefPtr <XmlResult> create (int status, const Glib::ustring & message,
                                            const std::vector<Glib::ustring>& return_values);

    Glib::ustring to_dest ();
    Glib::ustring to_body ();
    Glib::ustring to_xml ();
    int get_status();
    void dump_params();
    void dump();

    void setRawResult(const Glib::ustring &raw_result);

    const XmlParameterList& getParameterList() const
    {
        return _params;
    }

    XmlParameterList& getParameterList()
    {
        return _params;
    }

    void set_redirected () {
	_redirected = true;
    }

protected:
    int _status;
    XmlParameterList _params;
    Glib::ustring _message;
    Glib::ustring _textbody;
    Glib::ustring _raw_result;
    std::vector<Glib::ustring> _return_values;
    bool _redirected;
};

#endif
