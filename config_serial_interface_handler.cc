#include <map>
#include <cstring>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "config_serial_interface_handler.h"
#include "serial_helper.h"
#include "conf_handler.h"
#include "utils.h"

void ConfigSerialInterfaceHandler::set_port_configuration()
{
    auto conf = ConfHandler::get_instance();
    auto baud = SerialHelper::get_baudrate_for_parameter(parameterPrefix + "Baudrate");

    // get file descriptor for device
    auto input_file_fd = open(_device_name.c_str(), O_RDWR | O_NONBLOCK);
    if (input_file_fd < 0) {
        PRINT_ERROR("Failed to open device " << _device_name << ". Skipping configuration.");
        return;
    }

    // set the other settings (in this case, 9600 8N1)
    struct termios settings;
    tcgetattr(input_file_fd, &settings);

    // baud rate
    if (cfsetospeed(&settings, baud))
        PRINT_ERROR("Failed to set baudrate " << baud << ": " << strerror(errno));

    // parity, default: none
    auto serial_parity = conf->getParameter(parameterPrefix + "Parity");
    if (serial_parity == "none" || serial_parity == "") {
        settings.c_cflag &= ~PARENB; // no parity

	settings.c_iflag |= IGNPAR;
	settings.c_iflag &= ~INPCK;
	settings.c_iflag &= ~PARMRK;
    } else if (serial_parity == "odd") {
        settings.c_cflag |= PARENB; // parity
        settings.c_cflag |= PARODD; // odd parity

	settings.c_iflag &= ~IGNPAR;
	settings.c_iflag |= INPCK;
	settings.c_iflag |= PARMRK;
    } else if (serial_parity == "even") {
        settings.c_cflag |= PARENB; // parity
        settings.c_cflag &= ~PARODD; // even parity

	settings.c_iflag &= ~IGNPAR;
	settings.c_iflag |= INPCK;
	settings.c_iflag |= PARMRK;
    }

    // stop bits, default: 1
    auto stop_bits = conf->getParameter(parameterPrefix + "NstopBits");
    if (stop_bits == "1" || stop_bits == "")
        settings.c_cflag &= ~CSTOPB; // 1 stop bit
    else if (stop_bits == "2")
        settings.c_cflag |= CSTOPB; // 2 stop bits
    else
        settings.c_cflag &= ~CSTOPB; // 1 stop bit

    // data bits, default: 7
    settings.c_cflag &= ~CSIZE;
    auto data_bits = conf->getParameter(parameterPrefix + "NdataBits");
    if (data_bits == "7")
        settings.c_cflag |= CS7;
    else if (data_bits == "5")
        settings.c_cflag |= CS5;
    else if (data_bits == "6")
        settings.c_cflag |= CS6;
    else if (data_bits == "8")
        settings.c_cflag |= CS8;
    else
        settings.c_cflag |= CS8;

    auto handshaking = conf->getParameter(parameterPrefix + "Handshaking");
    if (handshaking == "none" || handshaking == "") {
        settings.c_cflag &= ~CRTSCTS;
	settings.c_iflag &= ~IXOFF;
	settings.c_iflag &= ~IXON;
    } else if (handshaking == "hwRtsCts") {
        settings.c_cflag |= CRTSCTS;
	settings.c_iflag &= ~IXOFF;
	settings.c_iflag &= ~IXON;
    } else if (handshaking == "swXonXoff") {
        settings.c_cflag &= ~CRTSCTS;
	settings.c_iflag |= IXOFF;
	settings.c_iflag |= IXON;
    }


    settings.c_cflag |= CLOCAL;  // modem control
    settings.c_lflag &= ~ICANON; // canonical mode
    settings.c_oflag &= ~OPOST;  // raw output
    settings.c_lflag &= ~ECHO;   // no echo of course
    settings.c_iflag &= ~ICRNL;  // no translation of carriage return

    // apply the settings
    if (tcsetattr(input_file_fd, TCSANOW, &settings))
        PRINT_ERROR("Failed to set serial port properties for device " << _device_name << ": " << strerror(errno));
    tcflush(input_file_fd, TCOFLUSH);

    // Note: We must close the fd. The serial perl scripts will use it.
    close(input_file_fd);

    return;
}

void ConfigSerialInterfaceHandler::slot_conf_change_announce(
    const Glib::ustring& par_id, const Glib::ustring& value, int &handlerMask)
{
    UNUSED(value);

    if ( ( par_id == parameterPrefix + "Baudrate" )
         || ( par_id == parameterPrefix + "Parity" )
         || ( par_id == parameterPrefix + "NdataBits" )
         || ( par_id == parameterPrefix + "NstopBits" )
         || ( par_id == parameterPrefix + "Handshaking" )
         || ( par_id == "all") )
        handlerMask |= myMask;
}

void ConfigSerialInterfaceHandler::slot_conf_changed( int handlerMask )
{
    if( handlerMask & myMask )
        set_port_configuration();
}
