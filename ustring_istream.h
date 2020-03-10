#ifndef ZIX_USTRING_ISTREAM
#define ZIX_USTRING_ISTREAM
//-----------------------------------------------------------------------------
///
/// \brief  ustring to istream converter
///
/// \date   [20161128] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


#include <glibmm/refptr.h>
#include <glibmm/bytes.h>

#include <streambuf>
#include <list>

/**
 * /brief allows to make std::istream from std::list <Glib::RefPtr <Glib::Bytes> >
 *
 * std::streambuf is used by std::istream (also std::ostream, but we only handle
 * std::istream here). We map the list of bytes, we collected earlier here.
 *
 * This allows us to hand over the bytes to libxml++
 *
 * \example
 *
 * UStringIStream is_buf (bytes_list);
 * std::istream is (&is_buf);
 */
class UStringIStream : public std::streambuf
{
    public:
        UStringIStream (Glib::ustring __ustring);

    protected:
	/**
	 * this function needs to be overridden.
	 * Its called, when the stream is at the end of the current buffer.
	 * We just switch the buffer pointers to the next Element in \a _data_list
	 */
	int underflow ();

    private:
        Glib::ustring _ustring;
};

//---fin-----------------------------------------------------------------------
#endif // ? ! ZIX_USTRING_ISTREAM
