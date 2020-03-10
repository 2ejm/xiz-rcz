
#ifndef ZIX_FUNCTION_CALL_H
#define ZIX_FUNCTION_CALL_H

#include "glibmm/refptr.h"
#include "glibmm/object.h"

#include "xml_result.h"
#include <libxml++/parsers/domparser.h>

class FunctionCall : public Glib::Object
{
    public:
	FunctionCall ();
	~FunctionCall ();

	virtual void start_call () = 0;
	sigc::signal<void, Glib::RefPtr <XmlResult> > finished;
};

#endif
