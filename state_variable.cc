
#include "state_variable.h"

#include "xml_query_set_parameter.h"
#include "xml_query_replay_status.h"
#include "utils.h"

/* for global installation we currently skip usb watchdog, which makes problems
 */
#ifdef GLOBAL_INSTALLATION
#define NUM_ALL_STATEVARS 6
#else
#define NUM_ALL_STATEVARS 5
#endif

StateVariable * StateVariable::_master = 0;
std::set <StateVariable *> StateVariable::_submitted;

void
StateVariable::set_master (StateVariable *master)
{
    StateVariable::_master = master;
}

StateVariable::StateVariable (const Glib::ustring & name)
    : _name (name)
    , _current_state ("unknown")
    , _query_state ("unknown")
    , _pending_state ("unknown")
{ }

StateVariable::~StateVariable ()
{ }

Glib::RefPtr <XmlQuery>
StateVariable::create_replay_query ()
{
    /* first create list of all State Variables,
     * with master being the last
     */

    std::vector <Glib::ustring> ids;
    std::vector <Glib::ustring> values;

    for (StateVariable * var : StateVariable::_submitted) {
	/* skip master, we add it last
	 */
	if (var == StateVariable::_master)
	    continue;

	ids.push_back (var->_name);
	values.push_back (var->_current_state);
    }

    ids.push_back (StateVariable::_master->_name);
    values.push_back (StateVariable::_master->_current_state);

    return XmlQueryReplayStatus::create (ids, values);
}

void
StateVariable::set_state (const Glib::ustring & state)
{
    PRINT_DEBUG ("StateVariable::set_state " << _name << " " << state);

    if ((this == StateVariable::_master) && (state == "ready")) {
	/* this is the master StateVariable
	 * check, if everybody has already submitted
	 */
	StateVariable::_submitted.insert (this);

	if (StateVariable::_submitted.size () != NUM_ALL_STATEVARS) {
	    /* not everybody has submitted
	     * delay set_state ();
	     */
	    PRINT_DEBUG ("StateVariable::set_state (): delaying");
	    return;
	}
    }

    /* check, whether we actually have to do something.
     */
    if (state == _pending_state) {
	return;
    }

    _pending_state = state;

    if (_state_query) {
	/* there is currently a query in progress.
	 * we already setup _pending_state,
	 * so after that query is through, a new query will start.
	 */
	return;
    }

    /* no query in progress, we start one
     */

    _query_state = _pending_state;

    auto dc = QueryClient::get_instance ();
    auto xq = XmlQuerySetParameter::create (_name, _query_state);

    _state_query = dc->create_query (xq);
    _state_query->finished.connect (sigc::mem_fun (*this, & StateVariable::on_query_finish));
    dc->execute(_state_query);
}

void
StateVariable::set_state (const Glib::ustring & state, sigc::slot <void, Glib::RefPtr <XmlResult> > callback)
{
    if (_state_query) {
	/* there is already a query in progress, try to use _pending_callback
	 */
	if (_pending_callback) {
	    /* when pending callback is also full, we give up.
	     */
	    PRINT_ERROR ("Can not start callback based StateVariable::set_state, while another is running.");
	    return;
	} else {
	    /* set _pending_callback and set_state and things will work,
	     * we look at _pending_callback later
	     */
	    _pending_callback = callback;
	    set_state (state);
	    return;
	}
    }

    _query_done_connection = _query_done.connect (callback);

    set_state (state);
}

void
StateVariable::on_query_finish (const Glib::RefPtr <XmlResult> & result)
{
    PRINT_DEBUG ("query is finished " << _name << "state: " << _query_state);
    /* now check result
     */

    if (result->get_status () != 200) {
	if (_query_done_connection.connected ()) {
	    /* emit the error to the callback
	     */
	    _query_done.emit (result);
	    _query_done_connection.disconnect ();
	}
	/* on error, we only report/log it
	 */
	PRINT_ERROR ("Error setting StateVar result: " << result->to_xml());

	/* before exiting we reset _state_query
	*/
	_state_query.reset ();

	return;
    }

    /* successfully set the Parameter
     * promote the query_state to current_state
     */
    _current_state = _query_state;

    /* now call the callback, if needed
     * and disconnect afterwards
     */
    if (_query_done_connection.connected ()) {
	_query_done.emit (result);
	_query_done_connection.disconnect ();
    }

    /* callbacks emitted, we can now assume that we dont recurse
     */
    _state_query.reset ();

    if (_pending_state != _current_state) {
	/* when there is a pending state change,
	 * we start it now
	 */

	_query_state = _pending_state;

	auto dc = QueryClient::get_instance ();
	auto xq = XmlQuerySetParameter::create (_name, _query_state);

	if (_pending_callback) {
	    _query_done_connection = _query_done.connect (_pending_callback);
	}

	_state_query = dc->create_query (xq);
	_state_query->finished.connect (sigc::mem_fun (*this, & StateVariable::on_query_finish));
	dc->execute(_state_query);
    }

    if (this != StateVariable::_master) {
	/* register that this state has been set
	 * with the global framework.
	 */
	StateVariable::_submitted.insert (this);

	if (StateVariable::_submitted.size () == NUM_ALL_STATEVARS) {
	    StateVariable::_master->set_state ("ready");
	}
    }
}
