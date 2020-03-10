
#ifndef LIBZIX_GIO_ISTREAM_ADAPTER_H
#define LIBZIX_GIO_ISTREAM_ADAPTER_H

#include <glibmm/object.h>
#include <giomm/inputstream.h>

#include <sigc++/signal.h>

#include <list>

/**
 * /brief Read out Gio::InputStream asynchronously
 *
 * This class reads out a Gio::InputStream into a std::list
 * of Glib::RefPtr <Glib::Bytes>. Reads until EOF.
 * Then emits stream_read signal with Data as Parameter.
 */
class GioIstreamAdapter : public Glib::Object
{
    public:
	GioIstreamAdapter (Glib::RefPtr <Gio::InputStream> gio_istream);

	/**
	 * Construct an Object which reads out \a gio_istream
	 */
	static Glib::RefPtr <GioIstreamAdapter> create (Glib::RefPtr <Gio::InputStream> gio_istream);

	/**
	 * Signal is emitted, whenever a single request was received
	 */
	sigc::signal<void, std::list <Glib::RefPtr <Glib::Bytes> > > stream_read;

    /**
     * Signal is emitted, whenever EOF is read
     */
    sigc::signal<void> stream_eof;

    protected:
	std::list <Glib::RefPtr <Glib::Bytes> > _bytes_list;
	Glib::RefPtr <Gio::InputStream> _gio_istream;

    private:
	void received_bytes (Glib::RefPtr<Gio::AsyncResult>& result);

	int bytes_list_check_zero ();
	std::list <Glib::RefPtr <Glib::Bytes> > pop_bytes_list (uint32_t index);
};
#endif
