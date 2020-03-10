#ifndef _TIME_UTILITIES_H_
#define _TIME_UTILITIES_H_

#include <string>

class TimeUtilities
{
public:
    static std::string get_timestamp();

    static std::string get_timestamp_log_format();

    static bool set_date(const std::string& date);

    static bool set_time(const std::string& time);

private:
    TimeUtilities()
    {}
};

#endif /* _TIME_UTILITIES_H_ */
