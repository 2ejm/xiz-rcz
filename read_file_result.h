#ifndef _READ_FILE_RESULT_H_
#define _READ_FILE_RESULT_H_

#include <string>

#include <glibmm/object.h>
#include <glibmm/refptr.h>

class ReadFileResult : public Glib::Object
{
public:
    ReadFileResult() :
        _error{false}
    {}

    ReadFileResult(const std::string& content, bool error = false, const std::string& error_msg = "") :
        _content(content), _error_msg(error_msg), _error{error}
    {}

    inline static Glib::RefPtr<ReadFileResult>
    create(const std::string& content, bool error = false, const std::string& error_msg = "")
    {
        return Glib::RefPtr<ReadFileResult>(new ReadFileResult(content, error, error_msg));
    }

    inline const std::string& content() const
    {
        return _content;
    }

    inline std::string& content()
    {
        return _content;
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
    std::string _content;
    std::string _error_msg;
    bool _error;
};

#endif /* _READ_FILE_RESULT_H_ */
