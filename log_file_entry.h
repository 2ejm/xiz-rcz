#ifndef _LOG_FILE_ENTRY_H_
#define _LOG_FILE_ENTRY_H_

#include <string>
#include <map>

#include <glibmm/ustring.h>

#include "xml_parameter_list.h"

/**
 * This class represents one log file entry/line.
 */
class LogFileEntry
{
public:
    LogFileEntry()
    {}

    /**
     * Builds LogFileEntry from XML request.
     *
     * @param parameters xml parameters
     */
    explicit LogFileEntry(const XmlParameterList& parameters)
    {
        build_from_xml(parameters);
    }

    /**
     * Builds LogFileEntry from manually specified parameters.  Useful, if you
     * want to log something without creating an XML request.
     *
     * @param parameters parameters
     */
    explicit LogFileEntry(const std::map<std::string, Glib::ustring>& parameters)
    {
        build_from_hash(parameters);
    }

    inline const Glib::ustring& entry() const noexcept
    {
        return _entry;
    }

    inline Glib::ustring& entry() noexcept
    {
        return _entry;
    }

private:
    Glib::ustring _entry;

    void build_from_xml(const XmlParameterList& parameters);
    void build_from_hash(const std::map<std::string, Glib::ustring>& parameters);

    static Glib::ustring xml_fmt;
};

#endif /* _LOG_FILE_ENTRY_H_ */
