
#include "gio_istream_adapter.h"

#include <cassert>

GioIstreamAdapter::GioIstreamAdapter (Glib::RefPtr <Gio::InputStream> gio_istream)
    : Glib::ObjectBase (typeid (GioIstreamAdapter))
    , _gio_istream (gio_istream)
{
    _gio_istream->read_bytes_async (1024, sigc::mem_fun (*this, &GioIstreamAdapter::received_bytes));
}

Glib::RefPtr <GioIstreamAdapter>
GioIstreamAdapter::create (Glib::RefPtr <Gio::InputStream> gio_istream)
{
    return Glib::RefPtr <GioIstreamAdapter> (new GioIstreamAdapter (gio_istream));
}

/**
 * Check last element of _bytes_list for zero
 */
int
GioIstreamAdapter::bytes_list_check_zero ()
{
    gsize i;
    gsize size;

    if (_bytes_list.empty())
        return -1;

    uint8_t *ptr = (uint8_t *) _bytes_list.back()->get_data (size);

    for (i=0; i<size; i++) {
	if (ptr[i] == 0x0)
	    return i;
    }

    /* fallthrough,
     * did not find zero
     */
    return -1;
}

/**
 * Split last Element of _bytes_list, and return all but the new last Element.
 */
std::list <Glib::RefPtr <Glib::Bytes> >
GioIstreamAdapter::pop_bytes_list (uint32_t index)
{
    std::list <Glib::RefPtr <Glib::Bytes> > retval = _bytes_list;
    Glib::RefPtr <Glib::Bytes> last = retval.back();

    retval.pop_back();
    _bytes_list.clear();

    gsize last_size;
    const uint8_t * last_data = (const uint8_t *) last->get_data (last_size);

    if (index > 0) {
	auto new_last = Glib::Bytes::create (last_data, index);
	retval.push_back (new_last);
    }

    if (index != (last_size - 1)) {
	auto remain = Glib::Bytes::create (last_data + index + 1, last_size - index - 1);
	_bytes_list.push_back (remain);
    }

    return retval;
}

void
GioIstreamAdapter::received_bytes (Glib::RefPtr<Gio::AsyncResult>& result)
{
    int index;
    auto source =  Glib::RefPtr <Gio::InputStream>::cast_dynamic (result->get_source_object ());

    assert (source);

    Glib::RefPtr <Glib::Bytes> bytes = source->read_bytes_finish (result);

    if (!bytes) {
	return;
    }

    if (bytes->get_size() == 0) {
	/* end of file
	 * emit done signa
	 */
	stream_read.emit (_bytes_list);
    stream_eof.emit();
	return;
    }

    _bytes_list.push_back (bytes);

    /* now check whether we see a zero terminator,
     * and emit all bytes up to that zero
     *
     * rinse and repeat until we see no more zeros
     */
    while ((index = bytes_list_check_zero ()) != -1)
    {
	auto pop_list = pop_bytes_list (index);
	stream_read.emit (pop_list);
    }

    /* start another read
     */
    source->read_bytes_async (1024, sigc::mem_fun (*this, &GioIstreamAdapter::received_bytes));
}
