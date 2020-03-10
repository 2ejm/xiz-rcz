#ifndef _LAN_WATCHDOG_H_
#define _LAN_WATCHDOG_H_

#include <glibmm/object.h>
#include <glibmm/refptr.h>

#include <vector>
#include <string>

#include "watchdog.h"
#include "timer.h"

/**
 * \brief LanWatchdog.
 *
 * Handles Lan Shared Folder input in a periodic way which is specified by
 * a timeout.
 */
class LanWatchdog final : public Watchdog
{
public:
    static inline Glib::RefPtr<LanWatchdog> create (const std::string& input, const Timer& timeout)
    {
        return Glib::RefPtr<LanWatchdog>(new LanWatchdog(input, timeout));
    }

    virtual ~LanWatchdog()
    {}

private:
    LanWatchdog (const std::string& input, const Timer& timeout);

    bool timeout_handler();

    ZixInterface get_zix_interface() const override
    {
        return STR_ZIXINF_LANSHAREDFOLDER;
    }

    FileDestination get_file_destination() const noexcept override
    {
        return FileDestination::Dest::lan;
    }

    std::string get_root_folder() const override;
};

#endif /* _LAN_WATCHDOG_H_ */
