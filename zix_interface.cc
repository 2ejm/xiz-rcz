#include "zix_interface.h"

const ZixInterface::InfMap ZixInterface::inf_map = {
    { STR_ZIXINF_IPC,             ZixInterface::Inf::IPC },
    { STR_ZIXINF_LANWEBSERVICE,   ZixInterface::Inf::LANwebservice },
    { STR_ZIXINF_LANWEBSERVER,    ZixInterface::Inf::LANwebserver },
    { STR_ZIXINF_LANSHAREDFOLDER, ZixInterface::Inf::LANsharedfolder },
    { STR_ZIXINF_LANSOCKET,       ZixInterface::Inf::LANsocket },
    { STR_ZIXINF_LANPRINTER,      ZixInterface::Inf::LANprinter },
    { STR_ZIXINF_USB,             ZixInterface::Inf::USB },
    { STR_ZIXINF_COM1,            ZixInterface::Inf::COM1 },
    { STR_ZIXINF_COM2,            ZixInterface::Inf::COM2 },
};

const ZixInterface::PrioMap ZixInterface::prio_map = {
    { ZixInterface::Inf::IPC, 4 },
    { ZixInterface::Inf::LANwebservice, 2 },
    { ZixInterface::Inf::LANwebserver, 2 },
    { ZixInterface::Inf::LANsharedfolder, 1 },
    { ZixInterface::Inf::LANsocket, 2 },
    { ZixInterface::Inf::LANprinter, 0 },
    { ZixInterface::Inf::USB, 1 },
    { ZixInterface::Inf::COM1, 3 },
    { ZixInterface::Inf::COM2, 3 },
    { ZixInterface::Inf::Unknown, 0 },
};

const ZixInterface::ParMap ZixInterface::par_map = {
    { ZixInterface::Inf::IPC, "" },
    { ZixInterface::Inf::LANwebservice, "lanWebservice" },
    { ZixInterface::Inf::LANwebserver, "" },
    { ZixInterface::Inf::LANsharedfolder, "lanFolder" },
    { ZixInterface::Inf::LANsocket, "lanSocket" },
    { ZixInterface::Inf::LANprinter, "lanPrinter" },
    { ZixInterface::Inf::USB, "usbFolder" },
    { ZixInterface::Inf::COM1, "serial" },
    { ZixInterface::Inf::COM2, "" },
    { ZixInterface::Inf::Unknown, "" },
};

std::string ZixInterface::to_string() const noexcept
{
    switch (_inf) {
    case Inf::IPC:             return STR_ZIXINF_IPC;
    case Inf::LANwebservice:   return STR_ZIXINF_LANWEBSERVICE;
    case Inf::LANwebserver:    return STR_ZIXINF_LANWEBSERVER;
    case Inf::LANsharedfolder: return STR_ZIXINF_LANSHAREDFOLDER;
    case Inf::LANsocket:       return STR_ZIXINF_LANSOCKET;
    case Inf::LANprinter:      return STR_ZIXINF_LANPRINTER;
    case Inf::USB:             return STR_ZIXINF_USB;
    case Inf::COM1:            return STR_ZIXINF_COM1;
    case Inf::COM2:            return STR_ZIXINF_COM2;
    case Inf::Unknown:         return "unknown";
    }

    return "";
}

int ZixInterface::prio() const noexcept
{
    return prio_map.at(_inf);
}

std::string ZixInterface::get_config_par() const noexcept
{
    return par_map.at(_inf);
}

std::ostream& operator<< (std::ostream& out, const ZixInterface& inf)
{
    out << inf.to_string();
    return out;
}
