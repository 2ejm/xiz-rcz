#ifndef ZIX_QUERY_CLIENT_SERIAL_H
#define ZIX_QUERY_CLIENT_SERIAL_H
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

#include "query_client.h"
#include "serial_interface_handler.h"


//---Includes------------------------------------------------------------------


class QuerySerial : public Query
{
    private:
        static Glib::RefPtr<SerialInterface> handler;

    public:
	QuerySerial (Glib::RefPtr <XmlQuery> xq, int tid, bool use_timeout);
	virtual ~QuerySerial ();
};


class QueryClientSerial : public QueryClient
{
    public:
	QueryClientSerial ();
	virtual ~QueryClientSerial ();

	static Glib::RefPtr <QueryClient> create ();

	Glib::RefPtr <Query> create_query (Glib::RefPtr <XmlQuery> xq, bool use_timeout) override;
	void reset_connection () override;
	void on_reset_handled ();

    void execute(Glib::RefPtr<Query> query) override;

    void on_response_ready(const Glib::ustring& responseString, int tid);
    void on_query_sent(int tid);
    bool on_query_timeout(int tid);
    void on_query_error(int tid);

    private:
    static Glib::RefPtr<SerialInterface> handler;
	sigc::connection _reset_connection;
    sigc::connection _response_connection;
	int _query_tid;
};


//---fin.----------------------------------------------------------------------
#endif // ? ! ZIX_QUERY_CLIENT_SERIAL_H

