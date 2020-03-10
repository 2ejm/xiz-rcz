
#ifndef LIBZIX_FUNCTION_CALL_SIGCHECK_H
#define LIBZIX_FUNCTION_CALL_SIGCHECK_H

#include "core_function_call.h"

#include "xml_restriction.h"

#include "signature_check_request.h"
#include "signature_check_result.h"

class FunctionCallSigCheck : public FunctionCall
{
    public:
	FunctionCallSigCheck (Glib::RefPtr <FunctionCall> call,
				    Glib::RefPtr <SignatureCheckRequest> scr,
				    const Glib::ustring & interface);

	~FunctionCallSigCheck ();

	static Glib::RefPtr <FunctionCall> create (Glib::RefPtr <FunctionCall> call,
						   Glib::RefPtr <SignatureCheckRequest> scr,
						   const Glib::ustring & interface);

	void start_call ();

    private:
	Glib::RefPtr <FunctionCall> _call;
	Glib::RefPtr <SignatureCheckRequest> _scr;
	Glib::ustring _interface;

	void signature_check_finished(Glib::RefPtr<SignatureCheckResult> result);
	void main_call_finished (Glib::RefPtr <XmlResult> result);
};
#endif
