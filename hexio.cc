
#include "hexio.h"

void
hexdump_mem (const char *ptr, gsize size)
{
    unsigned int i;

    if (size == 0) {
	printf ("empty data packet received\n");
	return;
    }

    for (i=0; i<size; i++) {
	if ((i%16) == 0) {
	    printf ("%08x: ", i);
	}

	printf ("%02x ", (unsigned int) ptr[i]);

	if ((i%16) == 15) {
	    printf ("\n");
	}
    }

    printf ("\n");
}

void
hexdump_bytes (const Glib::RefPtr <Glib::Bytes> & bytes)
{
    unsigned int i;
    gsize size;
    uint8_t *ptr = (uint8_t *) bytes->get_data (size);

    if (size == 0) {
	printf ("empty data packet received\n");
	return;
    }

    for (i=0; i<size; i++) {
	if ((i%16) == 0) {
	    printf ("%08x: ", i);
	}

	printf ("%02x ", (unsigned int) ptr[i]);

	if ((i%16) == 15) {
	    printf ("\n");
	}
    }

    printf ("\n");
}

void hexdump_bytes_list (const std::list <Glib::RefPtr <Glib::Bytes> > & bl)
{
    printf ("hexdump_bytes_list ===========================================\n");
    for (auto bytes : bl) {
	printf ("dump bytes -------------------------------------------------------\n");
	hexdump_bytes (bytes);
    }
    printf ("done =========================================================\n");
}


void
hexdump_stream (std::istream & is)
{
    int i=0;

    printf ("hexdump_stream\n");
    while (true) {
	int c = is.get();

	if (c == std::char_traits<char>::eof()) {
	    break;
	}

	if ((i%16) == 0) {
	    printf ("%08x: ", i);
	}

	printf ("%02x ", (unsigned int) c);

	if ((i%16) == 15) {
	    printf ("\n");
	}

	i+=1;
    }

    printf ("\n");
}
