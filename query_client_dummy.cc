
#include "query_client_dummy.h"
#include "serial_interface_handler.h"

#include "xml_result_ok.h"
#include "xml_result_bad_request.h"

QueryDummy::QueryDummy (Glib::RefPtr <XmlQuery> xq, int tid, bool use_timeout)
    : Query (xq)
{
    _use_timeout = use_timeout;
    _tid = tid;
}

QueryDummy::~QueryDummy ()
{ }

QueryClientDummy::QueryClientDummy () :
    _query_id (1)
{ }

QueryClientDummy::~QueryClientDummy ()
{ }

Glib::RefPtr <QueryClient>
QueryClientDummy::create ()
{
    return Glib::RefPtr <QueryClient> (new QueryClientDummy ());
}

Glib::RefPtr <Query>
QueryClientDummy::create_query (Glib::RefPtr <XmlQuery> xq, bool use_timeout)
{
    int tid = _query_id;

    _query_id += 2;

    auto query = Glib::RefPtr <Query> (new QueryDummy (xq, tid, use_timeout));

    _query_map[tid] = query;

    return query;
}

void
QueryClientDummy::reset_connection ()
{
    /* No action needs to be taken for Dummy
     */

    resetted.emit ();
}

void
QueryClientDummy::execute(Glib::RefPtr<Query> query)
{
    printf ("QueryDummy::execute ()\n");
    printf ("-------------------------------------------------\n");
    printf ("%s\n", query->xml_query().c_str ());
    printf ("-------------------------------------------------\n");

    query->finished.emit(XmlResultOk::create(query->xml_query()));

    _query_map.erase(query->tid());
}
