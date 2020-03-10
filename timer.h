#ifndef _TIMER_H_
#define _TIMER_H_

#include <glibmm/main.h>
#include <glibmm/ustring.h>

#include <sigc++/slot.h>

#include <iostream>

/*
 * \brief Represents a <timer /> declaration used for timeouts.
 *
 * A timeout consists of a value and a unit.
 */
class Timer final
{
public:
    using String = Glib::ustring;

    friend std::ostream& operator<< (std::ostream& out, const Timer& timer);

    inline Timer() :
        _value{"0"}, _unit{"s"}
    {}

    inline Timer(const String& value, const String& unit) :
        _value{value}, _unit{unit}
    {}

    void register_timeout(const sigc::slot<bool>& slot,
                          int priority = Glib::PRIORITY_DEFAULT) const;

    void register_timeout_once(const sigc::slot<void>& slot,
                               int priority = Glib::PRIORITY_DEFAULT) const;

    inline const String& value() const noexcept
    {
        return _value;
    }

    inline String& value() noexcept
    {
        return _value;
    }

    inline const String& unit() const noexcept
    {
        return _unit;
    }

    inline String& unit() noexcept
    {
        return _unit;
    }

private:
    String _value;
    String _unit;

    unsigned timeout() const;
};

#endif /* _TIMER_H_ */
