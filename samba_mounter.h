#ifndef _SAMBA_MOUNTER_H_
#define _SAMBA_MOUNTER_H_

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <glibmm/object.h>
#include <sigc++/signal.h>

#include <string>

#include "process_request.h"
#include "process_result.h"
#include "conf_handler.h"
#include "state_variable.h"

class SambaMounter : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<SambaMounter>;

    static inline RefPtr get_instance()
    {
        if (!instance)
            instance = create();
        return instance;
    }

    inline bool is_mounted() const noexcept
    {
        return _is_mounted;
    }

    inline const std::string& mount_path() const noexcept
    {
        return _mount_path;
    }

    void mount();
    void umount(bool mount_after = false);
    void remount();

    /*
     * mount() works asynchronously. This signal is used for informing when the
     * mount process is ready.
     *
     * The bool parameter signals whether the mount was successful or not.
     */
    sigc::signal<void, bool> mount_ready;

private:
    static RefPtr instance;

    Glib::RefPtr<ProcessRequest> _mount_proc;
    Glib::RefPtr<ProcessRequest> _umount_proc;
    bool _is_mounted;
    std::string _mount_path;
    Glib::ustring _share;
    Glib::ustring _user;
    Glib::ustring _pass;

    StateVariable _samba_state;

    SambaMounter();

    static inline RefPtr create()
    {
        return RefPtr(new SambaMounter());
    }

    void on_conf_change_announce(const Glib::ustring& par_id, const Glib::ustring& value, int& handler_mask);
    void on_conf_changed(const int handlerMask);
    void on_mount_finished(const Glib::RefPtr<ProcessResult>& result);
    void on_umount_finished(const Glib::RefPtr<ProcessResult>& result, bool mount_after);
};

#endif /* _SAMBA_MOUNTER_H_ */
