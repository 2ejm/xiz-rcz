#ifndef _START_STOPPABLE_H_
#define _START_STOPPABLE_H_

#include <glibmm/ustring.h>

#include "zix_interface.h"
#include "process_request.h"
#include "process_result.h"

/*
 * \brief Provides service restarts for ZiX Interfaces.
 *
 * If an InterfaceHandler inherits from this class, its associated daemon gets
 * automatically stopped and started by this class. That's currently the case
 * for lan shared folder (samba) und lan webservice (httpd).
 */
class StartStoppable
{
public:
   explicit StartStoppable(ZixInterface inf, bool use_conf_signal = true);

   void start();
   void stop();
   void restart();

private:
    ZixInterface _inf;
    Glib::ustring _par_id;
    Glib::ustring _current_state;

    Glib::RefPtr<ProcessRequest> _proc;

    void process(const std::string& command);

    void on_config_changed(const Glib::ustring& par_id, const Glib::ustring& value, int &handlerMask);
    void on_proc_finish(const Glib::RefPtr<ProcessResult>& result);
};

#endif /* _START_STOPPABLE_H_ */
