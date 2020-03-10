
#include "ustring_istream.h"


UStringIStream::UStringIStream (Glib::ustring __ustring)
    : _ustring (__ustring)
{
    auto size = _ustring.bytes();
    char *dat_ptr = (char *) _ustring.data();

    setg (dat_ptr, dat_ptr, dat_ptr + size);
}

int
UStringIStream::underflow ()
{
	return std::char_traits<char>::eof();
}
