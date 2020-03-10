
#ifndef ZIX_QUERY_CLIENT_DUMMY_H
#define ZIX_QUERY_CLIENT_DUMMY_H

#include "query_client.h"

class QueryDummy : public Query
{
    public:
	QueryDummy (Glib::RefPtr <XmlQuery> xq, int tid, bool use_timeout);
	virtual ~QueryDummy ();
};

class QueryClientDummy : public QueryClient
{
    public:
	QueryClientDummy ();
	virtual ~QueryClientDummy ();

	static Glib::RefPtr <QueryClient> create ();

	Glib::RefPtr <Query> create_query (Glib::RefPtr <XmlQuery> xq, bool use_timeout) override;
	void reset_connection () override;

    void execute(Glib::RefPtr<Query> query) override;

private:
    int _query_id;
};
#endif
