#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

#include <glibmm/object.h>
#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include <string>
#include <vector>

#include "lan_folder_entry.h"
#include "input_processor.h"
#include "process_request.h"
#include "process_result.h"
#include "zix_interface.h"
#include "file_destination.h"

/**
 * \brief Base class for WatchDogs.
 *
 * Combines common functionality of LanWatchdog and UsbWatchdog.
 */
class Watchdog : public Glib::Object
{
public:
    using LanFolderList = std::vector<LanFolderEntry>;
    using InputProcessorList = std::vector<InputProcessor>;
    using RefPtr = Glib::RefPtr<Watchdog>;

    Watchdog(const std::string& resource, const std::string& input, bool activate = true);

    virtual ~Watchdog()
    {}

    inline void activate() noexcept
    {
        _activated = true;
    }

    inline void deactivate() noexcept
    {
        _activated = false;
    }

    inline const bool& activated() const noexcept
    {
        return _activated;
    }

    inline bool& activated() noexcept
    {
        return _activated;
    }

protected:
    bool _activated;
    bool _in_progress;
    std::string _resource;
    std::string _input_folder;
    LanFolderList _folder_list;
    LanFolderList::const_iterator _folder_it;
    Glib::RefPtr<ProcessRequest> _ip_proc;

    std::string build_regex_pattern() const;
    std::string get_log_file_name(const LanFolderEntry& entry) const;
    InputProcessorList get_input_processors() const;

    LanFolderList create_file_list(const std::string& folder) const;
    void sort_file_list(LanFolderList& list) const;

    std::string get_log_folder() const;

    void move_or_copy_xml_to_log_dir(const LanFolderEntry& entry);
    void forward_entry_to_zix(const LanFolderEntry& entry);
    void forward_entry_to_ip(const LanFolderEntry& entry, const InputProcessor& ip);
    void process_entry(const LanFolderEntry& entry);
    void next();

    void on_ip_proc_ready(const Glib::RefPtr<ProcessResult>& result);

    /**
     * Each watchdog should have a ZiXInterface assigned to it.
     */
    virtual ZixInterface get_zix_interface() const = 0;

    /**
     * Each watchdog should have FileDestination associated with it.
     */
    virtual FileDestination get_file_destination() const noexcept = 0;

    /**
     * Each watchdog should have a root folder. For LAN it's the samba share
     * path, for USB it's the mount path.
     *
     * @return root path
     */
    virtual std::string get_root_folder() const = 0;
};

#endif /* _WATCHDOG_H_ */
