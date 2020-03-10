#ifndef _ACCESSIBILITY_CHECKER_H_
#define _ACCESSIBILITY_CHECKER_H_

#include <glibmm/ustring.h>

#include <map>
#include <string>

#include "zix_interface.h"

class AccessibilityChecker
{
public:
    using AccessCheckMap = std::map<std::string, std::map<std::string, bool> >;

    bool can_be_accessed(ZixInterface inf, const Glib::ustring& fid) const noexcept;

private:
    static const AccessCheckMap access_check_map;
};

#endif /* _ACCESSIBILITY_CHECKER_H_ */
