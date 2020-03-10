#include <libxml++/parsers/domparser.h>

#include <glibmm/init.h>
#include <glibmm/ustring.h>
#include <giomm/init.h>

#include <iostream>
#include <istream>
#include <string>
#include <cstdlib>

#include "ustring_istream.h"
#include "utils.h"

/**
 * Test process of how the serial interface handler (usb and serial) process
 * input data. Quite useful to debug UTF8 issues.
 *
 * Execute like this: ./test_serial_utf8 < test.xml
 */
void test_serial_utf8()
{
    std::string input;
    ssize_t bytes_read = 0;

    // read from stdin until eof
    while (23) {
        char buf[4096];

        auto size = read(fileno(stdin), buf, sizeof(buf));
        if (size < 0)
            throw std::logic_error("read() failed");
        if (size == 0)
            break;
        input.insert(bytes_read, buf, size);
        bytes_read += size;
    }

    // do conversion analog to serial handlers
    Glib::ustring input_utf8(input);
    UStringIStream is_buf(input_utf8);
    std::istream is(&is_buf);

    // parse xml out of it: will throw exception if anything is wrong
    xmlpp::DomParser parser;
    parser.parse_stream(is);
}

int main(void)
{
    Glib::init();
    Gio::init();

    test_serial_utf8();

    return EXIT_SUCCESS;
}
