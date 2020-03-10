
#ifndef LIBZIX_INTERFACE_HANDLER_H
#define LIBZIX_INTERFACE_HANDLER_H

#include <glibmm/object.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

#include <sigc++/signal.h>

#include <map>
#include <list>
#include <memory>

#include "xml_processor.h"
#include "interface_connection.h"
#include "zix_interface.h"
#include "start_stoppable.h"

class XmlProcessor;
class InterfaceConnection;

/**
 * \brief Baseclass for Interface Handlers
 *
 * Emit signal, when an input stream is ready
 * to get parsed.
 *
 * Also has virtual Method to emit the XmlResult back.
 */
class InterfaceHandler : virtual public Glib::Object, public StartStoppable
{
public:
    InterfaceHandler (ZixInterface inf, const Glib::ustring &type_name, Glib::RefPtr <XmlProcessor> xml_processor);

    virtual ~InterfaceHandler();
    static Glib::RefPtr<InterfaceHandler> getInterface( const Glib::ustring &name );
    static void register_handler( Glib::RefPtr<InterfaceHandler> );

protected:
    void register_connection (std::shared_ptr <InterfaceConnection> conn);
    void deregister_connection(std::weak_ptr <InterfaceConnection> conn);

    ZixInterface _inf;
    Glib::ustring _name;
private:
    static std::map<Glib::ustring, Glib::RefPtr<InterfaceHandler> > interface_map;
    std::list <std::shared_ptr <InterfaceConnection> > _connection_list;
    Glib::RefPtr <XmlProcessor> _xml_processor;
};

#endif
