#ifndef _FOLDER_H_
#define _FOLDER_H_

#include <glibmm/ustring.h>

#include "timer.h"

/**
 * \brief Represents one folder as used in <output>/<input> tags in zixconf.xml
 */
class Folder
{
public:
    Folder()
    {}

    Folder(const Glib::ustring& id, const Glib::ustring& type,
           const Glib::ustring& format, const Glib::ustring& path) :
        _id{id}, _type{type}, _format{format}, _path{path}
    {}

    const Glib::ustring& id() const
    {
        return _id;
    }

    Glib::ustring& id()
    {
        return _id;
    }

    const Glib::ustring& type() const
    {
        return _type;
    }

    Glib::ustring& type()
    {
        return _type;
    }

    const Glib::ustring& format() const
    {
        return _format;
    }

    Glib::ustring& format()
    {
        return _format;
    }

    const Glib::ustring& path() const
    {
        return _path;
    }

    Glib::ustring& path()
    {
        return _path;
    }

    const Timer& timer() const
    {
        return _timer;
    }

    Timer& timer()
    {
        return _timer;
    }

private:
    Glib::ustring _id;
    Glib::ustring _type;
    Glib::ustring _format;
    Glib::ustring _path;
    Timer _timer;
};

#endif /* _FOLDER_H_ */
