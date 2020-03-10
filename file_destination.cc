#include "file_destination.h"

const FileDestination::DestMap FileDestination::file_dest_map = {
    { STR_FILEDEST_PRINTER,    FileDestination::Dest::printer },
    { STR_FILEDEST_SOCKET,     FileDestination::Dest::socket },
    { STR_FILEDEST_LAN_FOLDER, FileDestination::Dest::lan },
    { STR_FILEDEST_USB_FOLDER, FileDestination::Dest::usb },
    { STR_FILEDEST_SERIAL,     FileDestination::Dest::serial },
    { STR_FILEDEST_LOCALPRINTER, FileDestination::Dest::localprinter },
};

bool FileDestination::is_file_based_dest() const noexcept
{
    return _dest == Dest::lan || _dest == Dest::usb;
}

bool FileDestination::is_printer_dest() const noexcept
{
    return _dest == Dest::printer;
}

bool FileDestination::is_socket_or_com_dest() const noexcept
{
    return _dest == Dest::socket || _dest == Dest::serial;
}

bool FileDestination::is_local_printer_dest () const noexcept
{
    return _dest == Dest::localprinter;
}

std::string FileDestination::to_string() const noexcept
{
    switch (_dest) {
    case Dest::printer: return STR_FILEDEST_PRINTER;
    case Dest::lan:     return STR_FILEDEST_LAN_FOLDER;
    case Dest::usb:     return STR_FILEDEST_USB_FOLDER;
    case Dest::socket:  return STR_FILEDEST_SOCKET;
    case Dest::serial:  return STR_FILEDEST_SERIAL;
    case Dest::localprinter:  return STR_FILEDEST_LOCALPRINTER;
    case Dest::unknown: return "unknown";
    }

    return "";
}

std::ostream& operator<< (std::ostream& out, const FileDestination& dest)
{
    out << dest.to_string();
    return out;
}
