#include <glibmm/main.h>

#include <sigc++/signal.h>

#include <stdexcept>

#include "conf_handler.h"
#include "utils.h"

#include "lan_watchdog.h"

LanWatchdog::LanWatchdog( const std::string& input, const Timer& timeout)
    : Watchdog ("lanFolder",
	        input,
	        ConfHandler::get_instance ()->getBoolParameter ("lanFolder"))
{
    // register timeout
    timeout.register_timeout(sigc::mem_fun(*this, &LanWatchdog::timeout_handler));
    PRINT_DEBUG("Lan Watchdog timeout registered @ " << timeout);
}

std::string LanWatchdog::get_root_folder() const
{
    auto conf_handler = ConfHandler::get_instance();
    return conf_handler->getRootFolder("lanFolder");
}

bool LanWatchdog::timeout_handler()
{
    if (!_activated || _in_progress)
        return true;

    PRINT_DEBUG("Running lan watch dog for folder " << _input_folder);

    try {
        auto conf_handler = ConfHandler::get_instance();
        _folder_list = create_file_list(this->get_root_folder() + "/" + _input_folder);
        sort_file_list(_folder_list);

        if (_folder_list.empty())
            return true;

        _in_progress = true;
        _folder_it = _folder_list.cbegin();
        process_entry(*_folder_it);
    } catch (const std::exception& ex) {
        PRINT_ERROR(ex.what());
	_in_progress = false;
    }

    return true;
}
