#ifndef _CONFIG_SERIAL_INTERFACE_HANDLER_H_
#define _CONFIG_SERIAL_INTERFACE_HANDLER_H_
//-----------------------------------------------------------------------------
///
/// \brief  Serial interface handler
///
///         See implementation for further details
///
/// \date   [20161114] File created
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
///         Modified "Serial-Port-Configurator" for COM1
///         by (C) 2017 Kurt Kanzenbach <kurt@linutronix.de>
///
//-----------------------------------------------------------------------------

//---Includes------------------------------------------------------------------


//---General--------------------------

#include <string>

#include <glibmm/ustring.h>
#include <glibmm/object.h>
#include <glibmm/refptr.h>

//---Own------------------------------

#include "conf_handler.h"
#include "zix_interface.h"


//---Declaration---------------------------------------------------------------

/**
 * \brief This class has only one purpose: It configures the serial port COM1.
 *
 * COM1 isn't used by us. It's used by e.g. the serial Perl scripts. We
 * configure this interface by the values given in the configuration xml. This
 * class also listens for configuration changes on this serial device.
 *
 * Note: There is no need to inherit from InterfaceHandler, as this class will
 *       not emit XML requests.
 */
class ConfigSerialInterfaceHandler : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<ConfigSerialInterfaceHandler>;

    ConfigSerialInterfaceHandler(ZixInterface inf, const std::string& device_name, int _myMask, std::string _parameterPrefix)
        : Glib::Object()
        , _inf{inf}
        , _device_name{device_name}
        , myMask(_myMask)
        , parameterPrefix( _parameterPrefix )
    {
        auto conf_handler = ConfHandler::get_instance();
        set_port_configuration();
        conf_handler->confChangeAnnounce.connect(
            sigc::mem_fun(*this, &ConfigSerialInterfaceHandler::slot_conf_change_announce ));
        conf_handler->confChanged.connect(
            sigc::mem_fun(*this, &ConfigSerialInterfaceHandler::slot_conf_changed ));
    }

    static inline RefPtr create(ZixInterface inf, const std::string& device_name, int _myMask, std::string _parameterPrefix)
    {
        return RefPtr(new ConfigSerialInterfaceHandler(inf, device_name, _myMask, _parameterPrefix));
    }

private:
    ZixInterface _inf;
    std::string _device_name;
    int myMask;
    std::string parameterPrefix;

    /**
     * This function actually configures the serial port.
     */
    void set_port_configuration();

    /**
     * Listen on config changes e.g. change of serial baudrate for COM1.
     *
     * @param par_id resource e.g. baudrate
     * @param value  new value
     */
    void slot_conf_change_announce(const Glib::ustring& par_id, const Glib::ustring& value, int &handlerMask);
    void slot_conf_changed(int handlerMask);
};

//-----------------------------------------------------------------------------
#endif /* _CONFIG_SERIAL_INTERFACE_HANDLER_H_ */
