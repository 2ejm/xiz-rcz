#ifndef _WEBSERVICE_H_
#define _WEBSERVICE_H_

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <glibmm/object.h>

#include <string>

#include "conf_handler.h"
#include "process_request.h"
#include "start_stoppable.h"

class Webservice : public Glib::Object
                  ,public StartStoppable
{
public:
    using RefPtr = Glib::RefPtr<Webservice>;

    static inline RefPtr get_instance()
    {
        if (!instance)
            instance = create( "LAN:webservice" );
        return instance;
    }

private:
    static RefPtr instance;

    Glib::RefPtr<ProcessRequest> _sed_proc;
    Webservice(ZixInterface inf) :
        Glib::Object()
      ,StartStoppable(inf, false)
    {
        ConfHandler::get_instance()->confChangeAnnounce.connect(
            sigc::mem_fun(*this, &Webservice::on_conf_change_announce));
        ConfHandler::get_instance()->confChanged.connect(
            sigc::mem_fun(*this, &Webservice::on_conf_changed));

        // Start webservice on startup
        updateConfig();
    }

    static inline RefPtr create(ZixInterface inf)
    {
        return RefPtr(new Webservice(inf));
    }

    void updateConfig();

    void on_conf_change_announce(const Glib::ustring& par_id, const Glib::ustring& value, int &handlerMask);
    void on_conf_changed(const int handlerMask);
    void on_sed_finished(const Glib::RefPtr<ProcessResult>& result);
    //void on_umount_finished(const Glib::RefPtr<ProcessResult>& result);
};

#endif /* _SAMBA_MOUNTER_H_ */
