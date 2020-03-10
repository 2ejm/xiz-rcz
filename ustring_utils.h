#ifndef _USTRING_UTILS_H_
#define _USTRING_UTILS_H_

#include <glibmm/ustring.h>

class UstringUtils
{
public:
    static Glib::ustring trim(const Glib::ustring& input);

    static Glib::ustring strip_newlines_and_tabs(const Glib::ustring& input);

    static Glib::ustring condense_spaces(const Glib::ustring& input);

    static Glib::ustring remove_indent_from_lines(const Glib::ustring& input);

private:
    UstringUtils()
    {}
};

#endif /* _USTRING_UTILS_H_ */
