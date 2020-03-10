#include <sstream>

#include "timer.h"

#include "utils.h"

unsigned Timer::timeout() const
{
    std::stringstream ss(_value.raw());
    unsigned timeout;
    ss >> timeout;
    if (ss.fail() || timeout == 0)
        EXCEPTION("Failed to parse timer value " << _value);

    return timeout;
}

void Timer::register_timeout(const sigc::slot<bool>& slot, int priority) const
{
    if (_unit == "ms") {
        Glib::signal_timeout().connect(slot, timeout(), priority);
        return;
    }
    if (_unit == "s") {
        Glib::signal_timeout().connect_seconds(slot, timeout(), priority);
        return;
    }

    EXCEPTION("Cannot register timeout with unknown unit " << _unit);
}

void Timer::register_timeout_once(const sigc::slot<void>& slot, int priority) const
{
    if (_unit == "ms") {
        Glib::signal_timeout().connect_once(slot, timeout(), priority);
        return;
    }
    if (_unit == "s") {
        Glib::signal_timeout().connect_seconds_once(slot, timeout(), priority);
        return;
    }

    EXCEPTION("Cannot register timeout with unknown unit " << _unit);
}

std::ostream& operator<< (std::ostream& out, const Timer& timer)
{
    out << "Timer(" << timer._value << " " << timer._unit << ")";
    return out;
}
