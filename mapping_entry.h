#ifndef _MAPPING_ENTRY_H_
#define _MAPPING_ENTRY_H_

#include <glibmm/ustring.h>

class MappingEntry
{
public:
    using String = Glib::ustring;

    MappingEntry()
    {}

    MappingEntry(const String& id, const String& iid, const String& date,
                 const String& time, const String& tz) :
        _id{id}, _iid{iid}, _date{date}, _time{time}, _tz{tz}
    {}

    const String& id() const
    {
        return _id;
    }

    String& id()
    {
        return _id;
    }

    const String& iid() const
    {
        return _iid;
    }

    String& iid()
    {
        return _iid;
    }

    const String& date() const
    {
        return _date;
    }

    String& date()
    {
        return _date;
    }

    const String& time() const
    {
        return _time;
    }

    String& time()
    {
        return _time;
    }

    const String& tz() const
    {
        return _tz;
    }

    String& tz()
    {
        return _tz;
    }

private:
    String _id;
    String _iid;
    String _date;
    String _time;
    String _tz;
};

#endif /* _MAPPING_ENTRY_H_ */
