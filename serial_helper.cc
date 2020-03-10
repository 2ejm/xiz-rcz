#include "conf_handler.h"

#include "serial_helper.h"

const std::map<Glib::ustring, speed_t> SerialHelper::rates = {
    { "1200", B1200 },
    { "1800", B1800 },
    { "2400", B2400 },
    { "4800", B4800 },
    { "9600", B9600 },
    { "19200", B19200 },
    { "38400", B38400 },
    { "57600", B57600 },
    { "115200", B115200 },
    { "230400", B230400 },
    { "460800", B460800 },
    { "500000", B500000 },
    { "576000", B576000 },
    { "921600", B921600 },
    { "1000000", B1000000 },
};

speed_t SerialHelper::get_baudrate_for_parameter(const Glib::ustring& parameter)
{
    auto conf_handler = ConfHandler::get_instance();
    Glib::ustring unit, value;
    speed_t baud = 0;

    if (conf_handler->getConf(parameter, unit, value)) {
        auto it = rates.find(value);

        if (it == rates.end())
            PRINT_WARNING("Unknown baud rate: " << value.raw () << ". Using default of 115200");
        else
            baud = it->second;
    }

    if (!baud)
        baud = B115200;

    return baud;
}
