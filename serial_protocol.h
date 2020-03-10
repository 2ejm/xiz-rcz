#ifndef _SERIAL_PROTOCOL_H_
#define _SERIAL_PROTOCOL_H_

#include <glibmm/ustring.h>

#include <vector>

/**
 * \brief Represents one serial protocol within zixconf.xml
 */
class SerialProtocol
{
public:
    using SchemeList = std::vector<Glib::ustring>;

    SerialProtocol()
    {}

    SerialProtocol(const Glib::ustring& id, const Glib::ustring& code) :
        _id{id}, _code{code}
    {}

    SerialProtocol(const Glib::ustring& id, const Glib::ustring& code,
                   const SchemeList& schemes) :
        _id{id}, _code{code}, _schemes{schemes}
    {}

    const Glib::ustring& id() const noexcept
    {
        return _id;
    }

    Glib::ustring& id() noexcept
    {
        return _id;
    }

    const Glib::ustring& code() const noexcept
    {
        return _code;
    }

    Glib::ustring& code() noexcept
    {
        return _code;
    }

    const SchemeList& schemes() const noexcept
    {
        return _schemes;
    }

    SchemeList& schemes() noexcept
    {
        return _schemes;
    }

private:
    Glib::ustring _id;
    Glib::ustring _code;
    SchemeList _schemes;
};

#endif /* _SERIAL_PROTOCOL_H_ */
