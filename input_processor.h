#ifndef _INPUT_PROCESSOR_H_
#define _INPUT_PROCESSOR_H_

#include <glibmm/ustring.h>

#include <vector>

/**
 * \brief Defines an input processor config object as defined by zixconf.xml
 */
class InputProcessor
{
public:
    using ResourceList = std::vector<Glib::ustring>;

    InputProcessor()
    {}

    InputProcessor(const Glib::ustring& id, const Glib::ustring& type,
                   const Glib::ustring& code, const Glib::ustring& pattern,
                   const ResourceList& resources) :
        _id{id}, _type{type}, _code{code}, _pattern{pattern}, _resources{resources}
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

    const Glib::ustring& code() const
    {
        return _code;
    }

    Glib::ustring& code()
    {
        return _code;
    }

    const Glib::ustring& pattern() const
    {
        return _pattern;
    }

    Glib::ustring& pattern()
    {
        return _pattern;
    }

    const ResourceList& resources() const
    {
        return _resources;
    }

    ResourceList& resources()
    {
        return _resources;
    }

private:
    Glib::ustring _id;
    Glib::ustring _type;
    Glib::ustring _code;
    Glib::ustring _pattern;
    ResourceList _resources;
};

#endif /* _INPUT_PROCESSOR_H_ */
