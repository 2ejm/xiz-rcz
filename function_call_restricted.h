
#ifndef LIBZIX_FUNCTION_CALL_RESTRICTED_H
#define LIBZIX_FUNCTION_CALL_RESTRICTED_H

#include "core_function_call.h"

#include "xml_restriction.h"

#include "restriction_check_request.h"
#include "restriction_check_result.h"

class FunctionCallRestricted : public FunctionCall
{
    public:
	FunctionCallRestricted (Glib::RefPtr <FunctionCall> call,
				    Glib::RefPtr <RestrictionCheckRequest> rcr,
				    const Glib::ustring & interface);

	~FunctionCallRestricted ();

	static Glib::RefPtr <FunctionCall> create (Glib::RefPtr <FunctionCall> call,
						   Glib::RefPtr <RestrictionCheckRequest> rcr,
						   const Glib::ustring & interface);

	void start_call ();

    private:
	Glib::RefPtr <FunctionCall> _call;
	Glib::RefPtr <RestrictionCheckRequest> _rcr;
	Glib::ustring _interface;

	void restriction_check_finished(Glib::RefPtr<RestrictionCheckResult> result);
	void main_call_finished (Glib::RefPtr <XmlResult> result);
};
#endif
