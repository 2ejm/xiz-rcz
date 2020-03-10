
#include "query_client.h"

#include "conf_handler.h"
#include "serial_interface_handler.h"

#ifdef USE_DEVICECONTROLLER
    #include "query_client_serial.h"
#else
    #include "query_client_dummy.h"
#endif

#define SERIAL_QUERY_TIMEOUT (120)

Query::Query (Glib::RefPtr <XmlQuery> xq)
    : _xml_query (xq->to_xml ())
    , _use_timeout (true)
    , _tid (0)
{ }

Query::~Query ()
{ }

void Query::disconnect_signals()
{
    _timeout_connection.disconnect();
    _error_connection.disconnect();
}

void Query::set_timeout(sigc::slot<bool, int> timeout_slot)
{
    // Actually connect a timeout?
    if (!_use_timeout)
        return;

    auto conf_handler = ConfHandler::get_instance();
    auto parameter = conf_handler->getParameter("dcQueryTimeout");
    int timeout;

    if (parameter.empty()) {
        timeout = SERIAL_QUERY_TIMEOUT;
    } else {
        timeout = std::atoi(parameter.c_str());
        if (timeout <= 0)
            timeout = SERIAL_QUERY_TIMEOUT;
    }

    lDebug("Setup timeout for query[tid=%d]: %d [s]\n", _tid, timeout);
    _timeout_connection = Glib::signal_timeout().connect(
        sigc::bind<int>(timeout_slot, _tid), timeout * 1000);
}

void Query::set_error(Glib::RefPtr<SerialInterface> handler, sigc::slot<void, int> error_slot)
{
    lDebug("Setup error callback for query[tid=%d]\n", _tid);
    _error_connection = handler->connection_errored.connect(
        sigc::bind<int>(error_slot, _tid));
}

QueryClient::QueryClient ()
{ }

QueryClient::~QueryClient ()
{ }

Glib::RefPtr <QueryClient> QueryClient::instance;

Glib::RefPtr <QueryClient>
QueryClient::get_instance ()
{
    if (! instance)
        #ifdef USE_DEVICECONTROLLER
            instance = QueryClientSerial::create ();
        #else
            instance = QueryClientDummy::create ();
        #endif

    return instance;
}
