
#ifndef ZIX_CORE_FUNCTION_CALL_H
#define ZIX_CORE_FUNCTION_CALL_H

#include <map>
#include <list>

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <sigc++/signal.h>

#include "xml_parameter_list.h"
#include "xml_description.h"
#include "xml_result.h"

#include "function_call.h"

/**
 * \brief Baseclass for Code that can be called via xml Request
 *
 * This Class owns the mapping Table from fid to a Factory,
 * which creates the function Call object.
 *
 * Note that a Function Call is asynchronous. Its started,
 * and emits a signal when finished.
 */
class CoreFunctionCall : public FunctionCall
{
    public:
	CoreFunctionCall (const Glib::ustring & fid,
		          XmlParameterList parameters,
			  Glib::RefPtr <XmlDescription> description,
			  const Glib::ustring & textbody);

	virtual ~CoreFunctionCall ();

	typedef Glib::RefPtr <CoreFunctionCall> (*Factory) (XmlParameterList parameters,
							Glib::RefPtr <XmlDescription> description,
							const Glib::ustring & textbody,
							const xmlpp::Element * en);


	static Glib::RefPtr<FunctionCall> create (const Glib::ustring & fid,
						  XmlParameterList parameters,
						  Glib::RefPtr <XmlDescription> description,
						  const Glib::ustring & textbody,
						  const xmlpp::Element * en,
						  const Glib::ustring & interface,
                                                  const Glib::ustring & filename = "");

	static void register_factory (const Glib::ustring & fid, Factory factory);
	static void init ();
	void set_interface(const Glib::ustring& interface);
        void set_filename(const Glib::ustring& filename);

    protected:
	XmlParameterList _parameters;
	Glib::RefPtr <XmlDescription> _description;
	Glib::ustring _textbody;
	Glib::ustring _fid;
	Glib::ustring _interface;
        Glib::ustring _filename;

	void call_finished (Glib::RefPtr <XmlResult> result);

    private:
	static std::map <Glib::ustring, Factory> factory_map;
};

#endif
