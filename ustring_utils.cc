#include "ustring_utils.h"

Glib::ustring UstringUtils::trim(const Glib::ustring& input)
{
    Glib::ustring result;

    result.reserve(input.size());
    for (const auto& c : input)
        if (!Glib::Unicode::isspace(c))
            result.push_back(c);

    return result;
}

Glib::ustring UstringUtils::strip_newlines_and_tabs(const Glib::ustring& input)
{
    Glib::ustring result;

    // FIXME: utf8 support
    result.reserve(input.size());
    for (auto&& c : input) {
        if (c == '\n' || c == '\t')
            continue;
        result.push_back(c);
    }

    return result;
}

Glib::ustring UstringUtils::condense_spaces(const Glib::ustring& input)
{
    Glib::ustring result;

    // FIXME: utf8 support
    result.reserve(input.size());
    gunichar e = 0;
    for (auto&& c : input) {
        if (e == ' ' && c == ' ')
            continue;
        result.push_back(c);
        e = c;
    }

    return result;
}

Glib::ustring UstringUtils::remove_indent_from_lines(const Glib::ustring& input)
{
    Glib::ustring result;
    bool erase = true;

    result.reserve(input.size());
    for (const auto& c : input) {
        if (c == '\n')
            erase = true;
        if (erase && (c == ' ' || c == '\t'))
            continue;
        if (c != ' ' && c != '\n')
            erase = false;
        result.push_back(c);
    }

    return result;
}
