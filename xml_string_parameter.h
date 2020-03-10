
#ifndef ZIX_XML_STRING_PARAMETER
#define ZIX_XML_STRING_PARAMETER

#include "xml_parameter.h"

/**
 * \brief Special XmlParameter that is only a string
 *
 * String Parameters might also be Node Arguments.
 *
 * <function fid="bla" parm1="blub">
 *   <parm2 value="blub"/>
 *   <parm3>blub</parm3>
 * </function>
 * 
 * All these parms are XmlStringParameter.
 * They dont need to be registered to the XmlParameter Factory
 */
class XmlStringParameter : public XmlParameter
{
    public:
	XmlStringParameter (const Glib::ustring & name, const Glib::ustring & val);

	static Glib::RefPtr <XmlStringParameter> create (const Glib::ustring & name, const Glib::ustring & val);

	const Glib::ustring & get_str();

	void dump ();
	Glib::ustring to_xml () const;

    private:
	Glib::ustring _val;
};
#endif
