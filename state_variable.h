#ifndef STATE_VARIABLE_H
#define STATE_VARIABLE_H

#include <set>

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

#include "xml_result.h"
#include "query_client.h"

class StateVariable
{
    public:
	StateVariable (const Glib::ustring & name);
	~StateVariable ();

	void set_state (const Glib::ustring & state);
	void set_state (const Glib::ustring & state, sigc::slot <void, Glib::RefPtr <XmlResult> > callback);

	static void set_master (StateVariable *master);

	static Glib::RefPtr <XmlQuery> create_replay_query ();
    private:
	static std::set <StateVariable *> _submitted;
	static StateVariable *_master;

	Glib::ustring _name;
	Glib::ustring _current_state;
	Glib::ustring _query_state;
	Glib::ustring _pending_state;

	sigc::slot <void, Glib::RefPtr <XmlResult> > _pending_callback;

	sigc::signal <void, Glib::RefPtr <XmlResult> > _query_done;
	sigc::connection _query_done_connection;

	Glib::RefPtr <Query> _state_query;

	void on_query_finish (const Glib::RefPtr <XmlResult> & result);
};

#endif
