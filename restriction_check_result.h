#ifndef _RESTRICTION_CHECK_RESULT_H_
#define _RESTRICTION_CHECK_RESULT_H_

#include <glibmm/object.h>
#include <glibmm/refptr.h>

#include <string>

class RestrictionCheckResult : public Glib::Object
{
public:
    RestrictionCheckResult(bool result, const std::string& error_msg) :
        Glib::Object(), _result{result}, _error_msg{error_msg}
    {}

    static Glib::RefPtr<RestrictionCheckResult>
    create(bool result, const std::string& error_msg)
    {
        return Glib::RefPtr<RestrictionCheckResult>(
            new RestrictionCheckResult(result, error_msg));
    }

    bool result() const { return _result; }

    const std::string& error_msg() const { return _error_msg; }
    std::string& error_msg() { return _error_msg; }

private:
    bool _result;
    std::string _error_msg;
};

#endif /* _RESTRICTION_CHECK_RESULT_H_ */
