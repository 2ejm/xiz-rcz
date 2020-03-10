#ifndef _LAN_PROTOCOL_H_
#define _LAN_PROTOCOL_H_

#include <glibmm/ustring.h>

#include <vector>

/**
 * \brief Represents one lan protocol handler within zixconf.xml
 */
class LanProtocol
{
public:
    LanProtocol()
    {}

    LanProtocol(const Glib::ustring& id, const Glib::ustring& code) :
        _id{id}, _code{code}
    {}

    const Glib::ustring& id() const
    {
        return _id;
    }

    Glib::ustring& id()
    {
        return _id;
    }

    const Glib::ustring& code() const
    {
        return _code;
    }

    Glib::ustring& code()
    {
        return _code;
    }

private:
    Glib::ustring _id;
    Glib::ustring _code;
};

#endif /* _LAN_PROTOCOL_H_ */
