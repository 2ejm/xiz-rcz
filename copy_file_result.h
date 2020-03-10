#ifndef _COPY_FILE_RESULT_H_
#define _COPY_FILE_RESULT_H_

#include <string>

#include <glibmm/object.h>
#include <glibmm/refptr.h>

class CopyFileResult : public Glib::Object
{
public:
    CopyFileResult() :
        _error{false}
    {}

    CopyFileResult(bool error = false, const std::string& error_msg = "") :
        _error_msg(error_msg), _error{error}
    {}

    inline static Glib::RefPtr<CopyFileResult>
    create(bool error = false, const std::string& error_msg = "")
    {
        return Glib::RefPtr<CopyFileResult>(new CopyFileResult(error, error_msg));
    }

    inline const std::string& error_msg() const
    {
        return _error_msg;
    }

    inline std::string& error_msg()
    {
        return _error_msg;
    }

    inline bool error() const
    {
        return _error;
    }

private:
    std::string _error_msg;
    bool _error;
};

#endif /* _COPY_FILE_RESULT_H_ */
