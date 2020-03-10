
#ifndef ZIX_XML_EXCEPTION_H
#define ZIX_XML_EXCEPTION_H

#include <exception>

#include <glibmm/ustring.h>

/**
 * \brief Baseclass for all XmlExceptions
 */
class XmlException : public std::exception
{
    public:
	XmlException (const Glib::ustring & msg);

	const char * what () const noexcept;

    private:
	Glib::ustring _msg;
};

/**
 * \brief Exception is thrown, when fid is unknown
 *
 * When <function fid="bla"> is parsed, and bla is not
 * a registered function.
 */
class XmlExceptionUnknownFid : public XmlException
{
    public:
	XmlExceptionUnknownFid (const Glib::ustring & fid);
};

/**
 * \brief Unknown structured Parameter found
 *
 * When a structured Parameter is found, that is not registered.
 * The Parameter is also not convertible to a StringParameter
 */
class XmlExceptionInvalidParameter : public XmlException
{
    public:
	XmlExceptionInvalidParameter (const Glib::ustring & name);
};

/**
 * \brief Illegal Access of fid via invalid interface
 *
 * Not all fids are accessible via all interfaces.
 */
class XmlExceptionNoAccess : public XmlException
{
    public:
	XmlExceptionNoAccess (const Glib::ustring & fid, const Glib::ustring & interface);
};
#endif
