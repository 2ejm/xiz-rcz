#ifndef _WRITE_FILE_RESULT_H_
#define _WRITE_FILE_RESULT_H_

#include <string>

#include <glibmm/object.h>
#include <glibmm/refptr.h>

class WriteFileResult : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<WriteFileResult>;

    WriteFileResult() :
        _error{false}
    {}

    WriteFileResult(bool error = false, const std::string& error_msg = "") :
        _error_msg(error_msg), _error{error}
    {}

    inline static RefPtr create(bool error = false, const std::string& error_msg = "")
    {
        return RefPtr(new WriteFileResult(error, error_msg));
    }

    inline const std::string& error_msg() const noexcept
    {
        return _error_msg;
    }

    inline std::string& error_msg() noexcept
    {
        return _error_msg;
    }

    inline bool error() const noexcept
    {
        return _error;
    }

private:
    std::string _error_msg;
    bool _error;
};

#endif /* _WRITE_FILE_RESULT_H_ */
