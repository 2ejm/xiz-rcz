
/**
 * \brief Monitor for Disk Usage
 *
 * Measures Disk Usage
 */

#include "monitor_disk_usage.h"

#include "utils.h"

#include <sys/statvfs.h>


MonitorDiskUsage::MonitorDiskUsage (const std::string & path_fname)
    : Monitor ("DiskUsage " + path_fname)
    , _path_fname (path_fname)
{ }


Glib::RefPtr <Monitor>
MonitorDiskUsage::create (const std::string & path_fname)
{
    return Glib::RefPtr <Monitor> (new MonitorDiskUsage (path_fname));
}


void MonitorDiskUsage::updateValues() /*virtual*/
{
    struct statvfs sfs;

    if (statvfs (_path_fname.c_str (), &sfs)) {
	/* statvfs did not return 0,
	 * error occured, oh well...
	 */
	PRINT_ERROR ("MonitorDiskUsage::updateValues(): statvfs failed");
	_disk_usage_percent = 0;
	return;
    }

    if (sfs.f_blocks == 0) {
	PRINT_ERROR ("MonitorDiskUsage::updateValues(): f_blocks is 0, can not calculate ratio");
	_disk_usage_percent = 0;
	return;
    }

    /* calclulate _disk_usage_percent
     */
    _disk_usage_percent = sfs.f_bavail * 100 / sfs.f_blocks;
    _disk_usage_percent = 100 - _disk_usage_percent;

    setValid(true);
}


Glib::ustring
MonitorDiskUsage::getLogString()
{
    return Glib::ustring::compose ("The current DiskUsage for path %1 is %2%%",
				   _path_fname,
                                   Glib::ustring::format (_disk_usage_percent));
}

long MonitorDiskUsage::getTriggerValue()
{
    return _disk_usage_percent;
}


//---fin.----------------------------------------------------------------------
