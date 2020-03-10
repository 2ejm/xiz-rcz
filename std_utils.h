#ifndef _STD_UTILS_H_
#define _STD_UTILS_H_

#include <string>
#include <sstream>

class StdUtils
{
public:
    template<typename T>
    static inline std::string join(T&& container)
    {
        std::stringstream ss{""};
        for (auto&& e : container)
            ss << e << " ";
        return ss.str();
    }

private:
    StdUtils()
    {}
};

#endif /* _STD_UTILS_H_ */
