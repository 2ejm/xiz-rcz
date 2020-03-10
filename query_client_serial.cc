//-----------------------------------------------------------------------------
///
/// \brief  query client serial
///
///         see implemention for further details
///
/// \date   [20161223] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

//---Own------------------------------

#include "query_client_serial.h"
#include "serial_interface_handler.h"
#include "usbserial_interface_handler.h"

#include "xml_result_ok.h"
#include "xml_result_parsed.h"
#include "xml_result_bad_request.h"
#include "xml_result_timeout.h"

#include "zix_interface.h"
#include "log.h"
#include "utils.h"
#include "conf_handler.h"

#define SERIAL_QUERY_TIMEOUT (120)

//---Implementation------------------------------------------------------------


Glib::RefPtr<SerialInterface> QueryClientSerial::handler; /* static */

QuerySerial::QuerySerial (Glib::RefPtr <XmlQuery> xq, int tid, bool use_timeout)
    : Query (xq)
{
    _use_timeout = use_timeout;
    _tid = tid;
}

QuerySerial::~QuerySerial ()
{ }

QueryClientSerial::QueryClientSerial ()
    : _query_tid (1)
{
    // get serial handler
    if(!handler)
    {
        // Search for the serial handler first.
        Glib::RefPtr<InterfaceHandler> _handler=InterfaceHandler::getInterface( STR_ZIXINF_IPC );
        if( !_handler )
            lInfo("Error: could not get serial handler\n");
        else
        {
            lInfo("Found serial handler\n");
            handler=Glib::RefPtr<SerialInterface>::cast_dynamic(_handler);
            if(!handler)
                lError("Serial handler not castable to SerialInterface\n");
        }
    }

    // connect incomming responses
    _response_connection = handler->response_ready.connect(
        sigc::mem_fun(*this, &QueryClientSerial::on_response_ready));

    /*
     * Connect query sent callback.
     *
     * We have to configure the timeout *after* the query has been
     * transmitted completely by the serial handler. sendMessage() works
     * asynchronous in the RS422 case.
     */
    handler->request_sent.connect(
        sigc::mem_fun(*this, &QueryClientSerial::on_query_sent));
}

QueryClientSerial::~QueryClientSerial ()
{
    // disconnect incomming responses
    _response_connection.disconnect();
}


Glib::RefPtr <QueryClient>
QueryClientSerial::create ()
{
    return Glib::RefPtr <QueryClient> (new QueryClientSerial ());
}


Glib::RefPtr <Query>
QueryClientSerial::create_query (Glib::RefPtr <XmlQuery> xq, bool use_timeout)
{
    int tid = _query_tid;

    /* task id is incremented by 2,
     * because odd taskids originate from sic.
     * even from dc
     */
    _query_tid += 2;

    auto query = Glib::RefPtr <Query> (new QuerySerial (xq, tid, use_timeout));

    _query_map[tid] = query;

    return query;
}

void
QueryClientSerial::reset_connection ()
{
    // Search for the serial handler first.
    Glib::RefPtr<InterfaceHandler> handler=InterfaceHandler::getInterface( STR_ZIXINF_IPC );
    if( !handler ) {
	lError("QueryClientSerial::reset_connection could not get serial handler\n");
	return;
    }

    Glib::RefPtr<SerialInterface> ser_interface =
	Glib::RefPtr<SerialInterface>::cast_dynamic(handler);
    if(!ser_interface) {
	lError("Serial handler not castable to SerialInterface\n");
	return;
    }

    _reset_connection = ser_interface->resetted.connect(
	    sigc::mem_fun(*this, &QueryClientSerial::on_reset_handled));

    ser_interface->reset_connection ();
}

void
QueryClientSerial::on_reset_handled ()
{
    _reset_connection.disconnect ();
    resetted.emit ();
}

void
QueryClientSerial::execute(Glib::RefPtr<Query> query)
{
    lDebug("QueryClientSerial::execute ()\n");
    lDebug ("%s\n", query->xml_query().c_str ());

    if (!handler) {
        lError("No serial handler available\n");
        query->finished.emit(XmlResultBadRequest::create());
        _query_map.erase(query->tid());
        return;
    }

    try {
        /* connect error and conditionally timeout callback
         */
        query->set_error(
            handler, sigc::mem_fun(*this, &QueryClientSerial::on_query_error));

        /* and then send the message
         */
        handler->sendMessage(
            Glib::ustring::compose("<task tid=\"%1\">%2</task>",
                                   query->tid(), query->xml_query()),
            query->tid());
    } catch (UsbInterfaceOffline &u) {
        query->finished.emit(XmlResultBadRequest::create(u.what()));
        _query_map.erase(query->tid());
        query->disconnect_signals();
    }
}

void
QueryClientSerial::on_query_sent(int tid)
{
    lDebug("QueryClientSerial::on_query_sent(tid=%d)\n", tid);

    auto it = _query_map.find(tid);
    if (it == std::end(_query_map)) {
        /* this does not seem to be for us
         * we just ignore this callback.
         */
        lDebug("Couldn't find query with TID: %d. Won't setup timeout.\n", tid);
        return;
    }

    auto query = it->second;

    /* guiIO calls may have unlimited timeouts.
     *
     * We can not specify a proper timeout for an interaction
     * with the user. Discussed with Dr. Buehrle, that this
     * behaviour is better than havin timeout.
     */
    query->set_timeout(sigc::mem_fun(*this, &QueryClientSerial::on_query_timeout));
}

void
QueryClientSerial::on_response_ready(const Glib::ustring& responseString, int tid)
{
    lDebug("QueryClientSerial::on_response_ready(tid=%d)\n", tid);
    lDebug("%s\n", responseString.c_str ());

    auto it = _query_map.find(tid);
    if (it == std::end(_query_map)) {
        /* this does not seem to be for us
         * we just ignore this callback.
         */
        lDebug("Received response with a unknown TID: %d\n", tid);
        return;
    }

    auto query = it->second;
    _query_map.erase(it);

    query->disconnect_signals();

    query->finished.emit(XmlResultParsed::create(responseString));
}

bool
QueryClientSerial::on_query_timeout(int tid)
{
    lDebug("QueryClientSerial::on_query_timeout(tid=%d)\n", tid);

    auto it = _query_map.find(tid);
    if (it == std::end(_query_map)) {
        /* this does not seem to be for us
         * we just ignore this callback.
         */
        lDebug("Received timeout with a unknown TID: %d\n", tid);
        return false;
    }

    auto query = it->second;
    _query_map.erase(it);

    /* when a timeout happens, we disconnect the signals.
     * then emit the timeout
     */

    query->disconnect_signals();

    query->finished.emit(XmlResultTimeout::create("DC Query Timeout"));

    /* timeout dont call again
     */
    return false;
}

void
QueryClientSerial::on_query_error(int tid)
{
    /* when an error happens, we disconnect all callbacks,
     * then emit an Error
     */

    lDebug("QuerySerial::on_error(tid=%d)\n", tid);

    auto it = _query_map.find(tid);
    if (it == std::end(_query_map)) {
        /* this does not seem to be for us
         * we just ignore this callback.
         */
        lDebug("Received error with a unknown TID: %d\n", tid);
        return;
    }

    auto query = it->second;
    _query_map.erase(it);

    query->disconnect_signals();

    query->finished.emit(XmlResultInternalDeviceError::create("Connection in error state"));
}

//-----------------------------------------------------------------------------
