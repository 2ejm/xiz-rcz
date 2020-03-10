#ifndef _ZIX_INTERFACE_H_
#define _ZIX_INTERFACE_H_

#include <glibmm/ustring.h>

#include <iostream>
#include <string>
#include <map>

/**
 * Interface names. The names may change withing the next specification. That's
 * why this is the only place where the names are defined.
 */
#define STR_ZIXINF_IPC             "Internal IPC"
#define STR_ZIXINF_LANWEBSERVICE   "LAN:webservice"
#define STR_ZIXINF_LANWEBSERVER    "LAN:webserver"
#define STR_ZIXINF_LANSHAREDFOLDER "LAN:sharedFolder"
#define STR_ZIXINF_LANSOCKET       "LAN:socket"
#define STR_ZIXINF_LANPRINTER      "LAN:printer"
#define STR_ZIXINF_USB             "USB"
#define STR_ZIXINF_COM1            "COM1"
#define STR_ZIXINF_DEBUG           "Debug"
#define STR_ZIXINF_COM2            "COM2"

/**
 * \brief Holds all available ZIX interfaces.
 *
 * See function specification table @ Spec V2 page 95 and following.
 */
class ZixInterface
{
public:
    enum class Inf {
        IPC, LANwebservice, LANwebserver, LANsharedfolder,
        LANsocket, LANprinter, USB, COM1, COM2, Unknown
    };

    using InfMap  = std::map<std::string, Inf>;
    using PrioMap = std::map<Inf, int>;
    using ParMap  = std::map<Inf, std::string>;

    friend std::ostream& operator<< (std::ostream& out, const ZixInterface& inf);

    ZixInterface()  :
        _inf{Inf::Unknown}
    {}

    ZixInterface(Inf inf)  :
        _inf{inf}
    {}

    ZixInterface(const std::string& inf)
    {
        auto it = inf_map.find(inf);
        if (it == inf_map.end())
            _inf = Inf::Unknown;
        else
            _inf = it->second;
    }

    ZixInterface(const Glib::ustring& inf) :
        ZixInterface(inf.raw())
    {}

    ZixInterface(const char *inf) :
        ZixInterface(std::string(inf))
    {}

    std::string to_string() const noexcept;

    std::string get_config_par() const noexcept;

    int prio() const noexcept;

    bool operator== (Inf inf) const noexcept
    {
        return _inf == inf;
    }

    bool operator!= (Inf inf) const noexcept
    {
        return !(*this == inf);
    }

    bool operator== (ZixInterface inf) const noexcept
    {
        return _inf == inf._inf;
    }

    bool operator!= (ZixInterface inf) const noexcept
    {
        return !(*this == inf);
    }

private:
    static const InfMap inf_map;
    static const PrioMap prio_map;
    static const ParMap par_map;

    Inf _inf;
};

#endif /* _ZIX_INTERFACE_H_ */
