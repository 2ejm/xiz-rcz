
#include "function_call_sigcheck.h"

#include "xml_result_bad_request.h"

FunctionCallSigCheck::FunctionCallSigCheck (Glib::RefPtr <FunctionCall> call,
						Glib::RefPtr <SignatureCheckRequest> scr,
						const Glib::ustring & interface)
    : FunctionCall ()
    , _call (call)
    , _scr (scr)
    , _interface (interface)
{ }

Glib::RefPtr <FunctionCall>
FunctionCallSigCheck::create (Glib::RefPtr <FunctionCall> call,
				Glib::RefPtr <SignatureCheckRequest> scr,
				const Glib::ustring & interface)
{
    return Glib::RefPtr <FunctionCall> (new FunctionCallSigCheck (call, scr, interface));
}

FunctionCallSigCheck::~FunctionCallSigCheck ()
{ }

void
FunctionCallSigCheck::start_call ()
{
    _scr->finished.connect(sigc::mem_fun(*this, &FunctionCallSigCheck::signature_check_finished));
    _scr->start_check(_interface);
}

void
FunctionCallSigCheck::signature_check_finished(Glib::RefPtr<SignatureCheckResult> result)
{
    if (!result->result()) {
        // signature check failed
        auto xml_result = XmlResultBadRequest::create(
            Glib::ustring::compose(
                "Signature check failed: %1", result->error_msg()));
	finished.emit (xml_result);
        return;
    }

    _call->finished.connect (sigc::mem_fun (*this, &FunctionCallSigCheck::main_call_finished));
    _call->start_call();
}

void
FunctionCallSigCheck::main_call_finished (Glib::RefPtr <XmlResult> result)
{
    finished.emit (result);
}
