#ifndef _DISK_USAGE_MANAGER_H_
#define _DISK_USAGE_MANAGER_H_

#include <glibmm/refptr.h>
#include <glibmm/object.h>
#include <glibmm/ustring.h>

/*
 * This class provides methods to free up some diskspace.
 */
class DiskUsageManager : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<DiskUsageManager>;

    static RefPtr get_instance()
    {
        if (!instance)
            instance = create();
        return instance;
    }

    /*
     * This functions removes all measurement directories which are flagged for
     * deletion.
     */
    void remove_flagged_measurements() const;

    /**
     * Performs shortening of current log files.
     */
    void logs_truncate() const;

    inline unsigned number_of_measurements_max() const noexcept
    {
        return _number_of_measurements_max;
    }

    inline unsigned number_of_measurements_keep() const noexcept
    {
        return _number_of_measurements_keep;
    }

    inline unsigned number_of_log_max() const noexcept
    {
        return _number_of_log_max;
    }

    inline unsigned number_of_log_keep() const noexcept
    {
        return _number_of_log_keep;
    }

private:
    DiskUsageManager();

    static RefPtr instance;

    unsigned _number_of_measurements_max;
    unsigned _number_of_measurements_keep;
    unsigned _number_of_log_max;
    unsigned _number_of_log_keep;

    static inline RefPtr create()
    {
        return RefPtr(new DiskUsageManager());
    }

    void update_config();
    void get_config_value(const std::string& name);
    void on_config_change_announce(const Glib::ustring& par_id, const Glib::ustring& value, int &handlerMask);
    void on_config_changed(const int handlerMask);
};

#endif /* _DISK_USAGE_MANAGER_H_ */
