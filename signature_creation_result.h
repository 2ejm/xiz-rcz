#ifndef _SIGNATURE_CREATION_RESULT_H_
#define _SIGNATURE_CREATION_RESULT_H_

#include <glibmm/object.h>
#include <glibmm/refptr.h>

#include <string>

class SignatureCreationResult : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<SignatureCreationResult>;

    SignatureCreationResult(bool success, const std::string& error_msg) :
        Glib::Object(),

        _success{success}, _error_msg{error_msg}
    {}

    static inline RefPtr create(bool success, const std::string& error_msg)
    {
        return RefPtr(new SignatureCreationResult(success, error_msg));
    }

    bool success() const noexcept
    {
        return _success;
    }

    const std::string& error_msg() const noexcept
    {
        return _error_msg;
    }

    std::string& error_msg() noexcept
    {
        return _error_msg;
    }

private:
    bool _success;
    std::string _error_msg;
};

#endif /* _SIGNATURE_CREATION_RESULT_H_ */
