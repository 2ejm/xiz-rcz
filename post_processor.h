#ifndef _POSTPROCESSOR_H_
#define _POSTPROCESSOR_H_

//-----------------------------------------------------------------------------
///
/// \brief  Postprocessor
///
///         Stupid class holding some strings.
///         See implemention for further details.
///
/// \date   [20161208] File created
///
/// \author Maximilian Seesslen <maximilian.seesslen@linutronix.de>
///
//-----------------------------------------------------------------------------


//---Includes------------------------------------------------------------------


//---General--------------------------

#include <glibmm/ustring.h>

#include <vector>
#include <algorithm>

#include "format.h"
#include "utils.h"

//---Own------------------------------

//---Declaration---------------------------------------------------------------

class PostProcessor
{
public:
    using FormatList = std::vector<Format>;

    PostProcessor()
    {}

    PostProcessor(const Glib::ustring& id, const Glib::ustring& code,
                  const Glib::ustring& ext) :
        _id{id}, _code{code}, _ext{ext}
    {}

    const Glib::ustring& id() const
    {
        return _id;
    }

    Glib::ustring& id()
    {
        return _id;
    }

    const Glib::ustring& code() const
    {
        return _code;
    }

    Glib::ustring& code()
    {
        return _code;
    }

    const Glib::ustring& ext() const
    {
        return _ext;
    }

    Glib::ustring& ext()
    {
        return _ext;
    }

    const FormatList& formats() const
    {
        return _formats;
    }

    FormatList& formats()
    {
        return _formats;
    }

    const Format& get_format_by_name(const Glib::ustring& format_name) const
    {
        auto it = std::find_if(_formats.begin(), _formats.end(),
                               [&format_name] (const Format& format) -> bool
                               {
                                   return format_name == format.id();
                               });
        if (it == _formats.end())
            EXCEPTION("Couldn't find format " << format_name);

        return *it;
    }

private:
    Glib::ustring _id;
    Glib::ustring _code;
    Glib::ustring _ext;
    FormatList _formats;
};


//---fin.----------------------------------------------------------------------
#endif // ! ? _POSTPROCESSOR_H_
