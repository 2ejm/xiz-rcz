#ifndef _DATA_FREE_HANDLER_H_
#define _DATA_FREE_HANDLER_H_

#include <glibmm/refptr.h>

#include <sigc++/signal.h>

#include "monitor.h"
#include "monitor_directory.h"
#include "disk_usage_manager.h"
#include "utils.h"

/**
 * \brief This class handles measurement deletion.
 *
 * This class does nothing by itself, it contains a handler which can be
 * registered at the DirectoryMontior. If the monitor alarm triggers, then the
 * measurements, which are flagged for deletion (-> see datafree), are finally
 * deleted.
 *
 * This class acts like glue code for Directory Monitor and Disk Usage Manager.
 */
class DataFreeHandler : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<DataFreeHandler>;
    using MonPtr = Glib::RefPtr<Monitor>;

    static inline RefPtr create(const MonPtr& monitor)
    {
        return RefPtr(new DataFreeHandler(monitor));
    }

    inline void handle_alarm(EAlarmDirection alarm, Monitor& monitor)
    {
        UNUSED(monitor);

        PRINT_DEBUG("Directory Alarm");
        if (alarm == eAlarmDirectionNone || alarm == eAlarmDirectionDisappear)
            return;

        PRINT_DEBUG("Remove flagged measurements");
        auto disk_manager = DiskUsageManager::get_instance();
        disk_manager->remove_flagged_measurements();
    }

private:
    inline explicit DataFreeHandler(const MonPtr& monitor) :
        Glib::Object()
    {
        // connect to alarm
        monitor->alarm.connect(
            sigc::mem_fun(*this, &DataFreeHandler::handle_alarm));
    }
};

#endif /* _DATA_FREE_HANDLER_H_ */
