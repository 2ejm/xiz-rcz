

#ifndef LIBZIX_INTERFACE_CONNECTION_H
#define LIBZIX_INTERFACE_CONNECTION_H

#include <glibmm/object.h>
#include <glibmm/ustring.h>

#include <sigc++/signal.h>

/**
 * \brief Baseclass for Interface Connections
 *
 * Emit signal, when an input stream is ready
 * to get parsed.
 *
 * Also has virtual Method to emit the XmlResult back.
 */
class InterfaceConnection : virtual public Glib::Object
{
    public:
	InterfaceConnection (const Glib::ustring & resume_reply_file);

	sigc::signal <void, std::istream &, int> request_ready;
	sigc::signal <void, const Glib::ustring &, int> response_ready;
	sigc::signal <void> connection_errored;
	sigc::signal <void> connection_closed;

	virtual ~InterfaceConnection();
	virtual void emit_result (const Glib::ustring &result, int tid) = 0;
	virtual void cancel () = 0;

	const Glib::ustring & get_resume_reply_file () { return _resume_reply_file; }
        const Glib::ustring & get_filename () { return _filename; }

    protected:
        // original filename of xml request if comming from file interface
        Glib::ustring _filename;

    private:
	Glib::ustring _resume_reply_file;
};

#endif
