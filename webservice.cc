#include <vector>
#include <giomm/file.h>
#include <giomm/dataoutputstream.h>

#include "file_handler.h"
#include "utils.h"

#include "webservice.h"

Webservice::RefPtr Webservice::instance(nullptr);


void Webservice::updateConfig()
{
    auto conf_handler = ConfHandler::get_instance();
    auto enabled      = conf_handler->getParameter("lanWebservice");
    auto port         = conf_handler->getParameter("lanWebservicePort");
    auto user         = conf_handler->getParameter("lanWebserviceUser");

    try {
        PRINT_DEBUG("Update Config");

        Glib::ustring authReplace;

        #if defined ( GLOBAL_INSTALLATION )
            Glib::RefPtr<Gio::File> file=Gio::File::create_for_path("/etc/lighttpd/.htpasswd_ws");
        #else
            Glib::RefPtr<Gio::File> file=Gio::File::create_for_path("htpasswd_ws");
        #endif

        Glib::RefPtr<Gio::DataOutputStream> out = Gio::DataOutputStream::create(file->replace());

        if( ( ! user.empty() ) && ( user != "anonymous" ) )
        {
            int pos=0;
            for (auto e : user)
            {
                if (e == '\\')
                    user.replace(pos, 1, ":");
                pos++;
            }

            out->put_string( user );
            out->put_string( "\n" );
            authReplace="        \"mod_auth\",";
        }
        else
        {
            out->put_string( "\n" );
            authReplace="        # \"mod_auth\",";
        }

        std::ostringstream serverPortStrStr, authStrStr;
        serverPortStrStr << "s/server\\.port.*=.*/server\\.port                 = ";
        serverPortStrStr << port;
        serverPortStrStr << "/g";
        authStrStr << "s/.*\"mod_auth\",/";
        authStrStr << authReplace;
        authStrStr << "/g";
        std::vector<std::string> sed_args{ "/bin/sed",
            "-e", serverPortStrStr.str(),
            "-e", authStrStr.str(),
            "-i",
#if defined ( GLOBAL_INSTALLATION )
            "/etc/lighttpd/lighttpd_ws.conf"
#else
            "lighttpd_ws.conf"
#endif
      };

        _sed_proc = ProcessRequest::create(sed_args, ProcessRequest::DEFAULT_TIMEOUT);
        _sed_proc->finished.connect(
            sigc::mem_fun(*this, &Webservice::on_sed_finished));
        _sed_proc->start_process();
        PRINT_DEBUG("Update Config done");
    } catch (const std::exception& ex) {
        PRINT_ERROR("Failed to update webservice config: " << ex.what());
    }
    catch ( ... ) {
        PRINT_ERROR("Failed to update webservice config");
    }
}


void Webservice::on_sed_finished(const Glib::RefPtr<ProcessResult>& result)
{
    if (!result->success()) {
        PRINT_ERROR("Sed failed: " << result->error_reason());
        return;
    }

    auto conf_handler = ConfHandler::get_instance();
    auto enabled      = conf_handler->getParameter("lanWebservice");
    auto port        = conf_handler->getParameter("lanWebservicePort");

    if (port == "") {
	stop ();
        PRINT_INFO("No webservice port given. Not starting.");
        return;
    }

    if( enabled == "enabled" )
        restart();
    else
        stop();
}


void Webservice::on_conf_change_announce(const Glib::ustring& par_id, const Glib::ustring& value, int &handlerMask)
{
    (void)value;
    (void)handlerMask;

    if (   ( par_id == "lanWebservice" )
        || ( par_id == "lanWebservicePort" )
        || ( par_id == "lanWebserviceUser" )
	|| ( par_id == "all" ) )
        handlerMask |= HANDLER_MASK_WEBSERVICE;
}

void Webservice::on_conf_changed( const int handlerMask )
{
    if ( handlerMask & HANDLER_MASK_WEBSERVICE )
    {
        updateConfig();
    }
}
