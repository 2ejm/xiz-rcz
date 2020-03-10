#ifndef _SERIAL_HELPER_H_
#define _SERIAL_HELPER_H_

#include <glibmm/ustring.h>

#include <map>

#include <termios.h>

class SerialHelper
{
public:
    static speed_t get_baudrate_for_parameter(const Glib::ustring& parameter);

private:
    static const std::map<Glib::ustring, speed_t> rates;
};

#endif /* _SERIAL_HELPER_H_ */
