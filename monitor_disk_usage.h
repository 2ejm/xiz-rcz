#ifndef MONITOR_DISK_USAGE_H
#define MONITOR_DISK_USAGE_H

/**
 * \brief  Monitor for Disk usage
 *
 * Measures cpu usage.
 */

#include "monitor.h"


#include <string>
#include <fstream>
#include <iomanip>


class MonitorDiskUsage: public Monitor
{
    public:
        MonitorDiskUsage (const std::string & path_fname);
        static Glib::RefPtr <Monitor> create (const std::string &);

        virtual void updateValues();
        virtual Glib::ustring getLogString();

        virtual long getTriggerValue();

    private:

	std::string _path_fname;
	long _disk_usage_percent;
};

#endif // MONITOR_DISK_USAGE_H
