
#ifndef LIBZIX_HEXIO_H
#define LIBZIX_HEXIO_H

#include <glibmm/bytes.h>
#include <glibmm/refptr.h>

#include <istream>
#include <list>

void hexdump_mem (const char *ptr, gsize size);

/**
 * \brief Printf a hexdump from bytes.
 */
void hexdump_bytes (const Glib::RefPtr <Glib::Bytes> & bytes);

void hexdump_bytes_list (const std::list <Glib::RefPtr <Glib::Bytes> > & bl);

/**
 * \brief Printf a hexdump from std::istream
 */
void hexdump_stream (std::istream & is);
#endif
