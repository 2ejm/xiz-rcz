#ifndef ZIX_QUERY_CLIENT_H
#define ZIX_QUERY_CLIENT_H

#include <glibmm/object.h>
#include <glibmm/refptr.h>

#include <map>

#include "xml_query.h"
#include "xml_result.h"

class SerialInterface;

class Query : public Glib::Object
{
    public:
	Query (Glib::RefPtr <XmlQuery> xq);
	virtual ~Query ();

	sigc::signal <void, Glib::RefPtr <XmlResult> >  finished;

    const Glib::ustring& xml_query() const
    {
        return _xml_query;
    }

    int tid() const
    {
        return _tid;
    }

    void disconnect_signals();

    void set_timeout(sigc::slot<bool, int> timeout_slot);

    void set_error(Glib::RefPtr<SerialInterface> handler, sigc::slot<void, int> error_slot);

    protected:
	Glib::ustring _xml_query;
	bool _use_timeout;
    int _tid;
    sigc::connection _timeout_connection;
    sigc::connection _error_connection;
};

class QueryClient : public Glib::Object
{
    public:
	QueryClient ();
	virtual ~QueryClient ();

	virtual Glib::RefPtr <Query> create_query (Glib::RefPtr <XmlQuery> xq, bool use_timeout = true) = 0;

	static Glib::RefPtr <QueryClient> get_instance ();

	sigc::signal <void>  resetted;
	virtual void reset_connection () = 0;

    virtual void execute(Glib::RefPtr<Query> query) = 0;

protected:
    std::map<int, Glib::RefPtr<Query> > _query_map;

private:
	static Glib::RefPtr <QueryClient> instance;
};
#endif
