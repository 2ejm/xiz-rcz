#include <sstream>
#include <cmath>
#include <ctime>
#include <cstring>
#include <cerrno>

#include <sys/time.h>

#include "time_utilities.h"
#include "utils.h"

std::string TimeUtilities::get_timestamp()
{
    char str1[64], str2[64];
    timeval current;
    int millisec;

    gettimeofday(&current, NULL);

    // rounding
    millisec = lrint(current.tv_usec / 1000.0);
    if (millisec >= 1000) {
        millisec -= 1000;
        current.tv_sec++;
    }

    // example: 2016-11-19 19:31:02.154
    if (!std::strftime(str1, sizeof(str1), "%Y-%m-%d %H:%M:%S",
                       std::localtime(&current.tv_sec)))
        return "";

    snprintf(str2, sizeof(str2), "%s.%03d", str1, millisec);

    return str2;
}

std::string TimeUtilities::get_timestamp_log_format()
{
    char str1[64];
    timeval current;
    int millisec;

    gettimeofday(&current, NULL);

    // rounding
    millisec = lrint(current.tv_usec / 1000.0);
    if (millisec >= 1000) {
        millisec -= 1000;
        current.tv_sec++;
    }

    // example: 20161119_193102
    if (!std::strftime(str1, sizeof(str1), "%Y%m%d_%H%M%S",
                       std::localtime(&current.tv_sec)))
        return "";

    return str1;
}

bool TimeUtilities::set_date(const std::string& date)
{
    struct timeval current;

    // date: %Y-%m-%d
    int year, month, day;
    std::stringstream ss{date};

    if (!(ss >> year >> month >> day)) {
        PRINT_ERROR("Failed to parse date " << date);
        return false;
    }

    if (month < 0)
        month = -month;
    if (day < 0)
        day = -day;

    gettimeofday(&current, NULL);
    struct tm *time = gmtime(&current.tv_sec);

    time->tm_mday = day;
    time->tm_mon  = month - 1;
    time->tm_year = year - 1900;

    current.tv_sec  = mktime(time);
    current.tv_usec = 0;

    auto ret = settimeofday(&current, NULL);
    if (ret) {
        PRINT_ERROR("Failed to set date: " << strerror(errno));
        return false;
    }

    return true;
}

bool TimeUtilities::set_time(const std::string& input_time)
{
    struct timeval current;

    // time: %H:%M:%S
    int hours, minutes, seconds;
    std::stringstream ss{input_time};
    char sep;

    if (!(ss >> hours >> sep >> minutes >> sep >> seconds)) {
        PRINT_ERROR("Failed to parse time " << input_time);
        return false;
    }

    gettimeofday(&current, NULL);
    struct tm *time = gmtime(&current.tv_sec);

    time->tm_sec  = seconds;
    time->tm_min  = minutes;
    time->tm_hour = hours;

    current.tv_sec = mktime(time);
    current.tv_usec = 0;

    auto ret = settimeofday(&current, NULL);
    if (ret) {
        PRINT_ERROR("Failed to set date: " << strerror(errno));
        return false;
    }

    return true;
}
