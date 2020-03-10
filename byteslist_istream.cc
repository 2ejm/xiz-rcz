
#include "byteslist_istream.h"


BytesListIStream::BytesListIStream (std::list <Glib::RefPtr <Glib::Bytes> > data)
    : _data_list (data)
{
    auto curr = _data_list.begin();

    if (curr == _data_list.end()) {
	setg (0, 0, 0);
    } else {
	gsize size;
	char *dat_ptr = (char *) ((*curr)->get_data(size));

	setg (dat_ptr, dat_ptr, dat_ptr + size);
    }
}

int
BytesListIStream::underflow ()
{
    if (_data_list.size () == 0)
	return std::char_traits<char>::eof();

    _data_list.pop_front();

    auto curr = _data_list.begin();

    if (curr == _data_list.end()) {
	return std::char_traits<char>::eof();
    }

    gsize size;
    char * dat_ptr = (char *)((*curr)->get_data(size));

    setg (dat_ptr, dat_ptr, dat_ptr + size);

    return *dat_ptr;
}
