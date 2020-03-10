#ifndef _SIGNATURE_CHECK_RESULT_H_
#define _SIGNATURE_CHECK_RESULT_H_

#include <glibmm/object.h>
#include <glibmm/refptr.h>

#include <string>

class SignatureCheckResult : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<SignatureCheckResult>;

    SignatureCheckResult(bool result, const std::string& error_msg) :
        Glib::Object(),
        _result{result}, _error_msg{error_msg}
    {}

    static inline RefPtr create(bool result, const std::string& error_msg)
    {
        return RefPtr(new SignatureCheckResult(result, error_msg));
    }

    bool result() const
    {
        return _result;
    }

    const std::string& error_msg() const
    {
        return _error_msg;
    }

    std::string& error_msg()
    {
        return _error_msg;
    }

private:
    bool _result;
    std::string _error_msg;
};

#endif /* _SIGNATURE_CHECK_RESULT_H_ */
