#ifndef _LOG_TRUNCATE_HANDLER_H
#define _LOG_TRUNCATE_HANDLER_H

#include <glibmm/refptr.h>

#include <sigc++/signal.h>

#include "monitor.h"
#include "monitor_directory.h"
#include "disk_usage_manager.h"
#include "utils.h"

/**
 * \brief This class handles log truncate
 *
 * This class does nothing by itself, it contains a handler which can be
 * registered at the DiskUsageMonitor. If the monitor alarm triggers,
 * the log_truncating function is called.
 *
 * This class acts like glue code for Directory Monitor and Disk Usage Manager.
 */
class LogTruncateHandler : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<LogTruncateHandler>;
    using MonPtr = Glib::RefPtr<Monitor>;

    static inline RefPtr create(const MonPtr& monitor)
    {
        return RefPtr(new LogTruncateHandler(monitor));
    }

    inline void handle_alarm(EAlarmDirection alarm, Monitor& monitor)
    {
        UNUSED(monitor);

        PRINT_DEBUG("Log Truncate Alarm");
        if (alarm == eAlarmDirectionNone || alarm == eAlarmDirectionDisappear)
            return;

        PRINT_DEBUG("call log_truncate");

        auto disk_manager = DiskUsageManager::get_instance();
        disk_manager->logs_truncate();
    }

private:
    inline explicit LogTruncateHandler(const MonPtr& monitor) :
        Glib::Object()
    {
        // connect to alarm
        monitor->alarm.connect(
            sigc::mem_fun(*this, &LogTruncateHandler::handle_alarm));
    }
};

#endif /* _LOG_TRUNCATE_HANDLER_H */
