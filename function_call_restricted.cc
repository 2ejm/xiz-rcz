
#include "function_call_restricted.h"

#include "xml_result_bad_request.h"
#include "xml_result_ok.h"

FunctionCallRestricted::FunctionCallRestricted (Glib::RefPtr <FunctionCall> call,
						Glib::RefPtr <RestrictionCheckRequest> rcr,
						const Glib::ustring & interface)
    : FunctionCall ()
    , _call (call)
    , _rcr (rcr)
    , _interface (interface)
{ }

Glib::RefPtr <FunctionCall>
FunctionCallRestricted::create (Glib::RefPtr <FunctionCall> call,
				Glib::RefPtr <RestrictionCheckRequest> rcr,
				const Glib::ustring & interface)
{
    return Glib::RefPtr <FunctionCall> (new FunctionCallRestricted (call, rcr, interface));
}

FunctionCallRestricted::~FunctionCallRestricted ()
{ }

void
FunctionCallRestricted::start_call ()
{
    _rcr->finished.connect(sigc::mem_fun(*this, &FunctionCallRestricted::restriction_check_finished));
    _rcr->start_check(_interface);
}

void
FunctionCallRestricted::restriction_check_finished(Glib::RefPtr<RestrictionCheckResult> result)
{
    if (!result->result()) {
        /* restriction check failed
	 * however... this is not fatal
	 *
	 * we return XmlResultOk and dont
	 * start the main call.
	 */
        auto xml_result = XmlResultOk::create(
            Glib::ustring::compose(
                "Restriction check failed: %1", result->error_msg()));
	finished.emit (xml_result);
        return;
    }

    _call->finished.connect (sigc::mem_fun (*this, &FunctionCallRestricted::main_call_finished));
    _call->start_call();
}

void
FunctionCallRestricted::main_call_finished (Glib::RefPtr <XmlResult> result)
{
    finished.emit (result);
}
