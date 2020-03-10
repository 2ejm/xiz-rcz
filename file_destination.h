#ifndef _FILE_DESTINATION_H_
#define _FILE_DESTINATION_H_

#include <glibmm/ustring.h>

#include <iostream>
#include <string>
#include <map>

#define STR_FILEDEST_PRINTER    "lanPrinter"
#define STR_FILEDEST_SOCKET     "lanSocket"
#define STR_FILEDEST_LAN_FOLDER "lanFolder"
#define STR_FILEDEST_USB_FOLDER "usbFolder"
#define STR_FILEDEST_SERIAL     "serial"
#define STR_FILEDEST_LOCALPRINTER "localPrinter"

/**
 * \brief Holds all available ZIX file destinations.
 *
 * See e.g. DataOut or GetMeasurement.
 */
class FileDestination
{
public:
    enum class Dest {
        printer, socket, lan, usb, serial, localprinter, unknown
    };

    using DestMap = std::map<std::string, Dest>;

    friend std::ostream& operator<< (std::ostream& out, const FileDestination& dest);

    FileDestination() :
        _dest{Dest::unknown}
    {}

    FileDestination(Dest dest) :
        _dest{dest}
    {}

    FileDestination(const std::string& dest)
    {
        auto it = file_dest_map.find(dest);
        if (it == file_dest_map.end())
            _dest = Dest::unknown;
        else
            _dest = it->second;
    }

    FileDestination(const Glib::ustring& dest) :
        FileDestination(dest.raw())
    {}

    FileDestination(const char *dest) :
        FileDestination(std::string(dest))
    {}

    bool is_file_based_dest() const noexcept;
    bool is_printer_dest() const noexcept;
    bool is_local_printer_dest() const noexcept;
    bool is_socket_or_com_dest() const noexcept;

    std::string to_string() const noexcept;

    bool operator== (Dest dest) const noexcept
    {
        return _dest == dest;
    }

    bool operator!= (Dest dest) const noexcept
    {
        return !(*this == dest);
    }

private:
    static const DestMap file_dest_map;

    Dest _dest;
};

#endif /* _FILE_DESTINATION_H_ */
