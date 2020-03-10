#ifndef _FORMAT_H_
#define _FORMAT_H_

#include <glibmm/ustring.h>

/**
 * \brief Represents one <format> entry in zixconf.xml
 */
class Format
{
public:
    Format()
    {}

    Format(const Glib::ustring& id, const Glib::ustring& ext,
           const Glib::ustring& tag) :
        _id{id}, _ext{ext}, _tag{tag}
    {}

    const Glib::ustring& id() const
    {
        return _id;
    }

    Glib::ustring& id()
    {
        return _id;
    }

    const Glib::ustring& ext() const
    {
        return _ext;
    }

    Glib::ustring& ext()
    {
        return _ext;
    }

    const Glib::ustring& tag() const
    {
        return _tag;
    }

    Glib::ustring& tag()
    {
        return _tag;
    }

private:
    Glib::ustring _id;
    Glib::ustring _ext;
    Glib::ustring _tag;
};

#endif /* _FORMAT_H_ */
