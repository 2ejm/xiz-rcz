#include "log_file_entry.h"

#include "ustring_utils.h"
#include "xml_helpers.h"

#define MAX_LOG_ENTRY_SIZE (1 << 10)  // 1 KiB

Glib::ustring LogFileEntry::xml_fmt =
    "<message source=\"%1\" timestamp=\"%2\" category=\"%3\" level=\"%4\" theme=\"%5\">%6</message>\n";

void LogFileEntry::build_from_xml(const XmlParameterList& parameters)
{
    Glib::ustring xml_fmt =
        "<message source=\"%1\" timestamp=\"%2\" category=\"%3\" level=\"%4\" theme=\"%5\">%6</message>\n";

    Glib::ustring msg = parameters.get_str("message");

    if (msg.size() > MAX_LOG_ENTRY_SIZE)
        msg.erase(MAX_LOG_ENTRY_SIZE, msg.size() - MAX_LOG_ENTRY_SIZE);

    msg = UstringUtils::strip_newlines_and_tabs(msg);
    msg = UstringUtils::condense_spaces(msg);

    _entry = Glib::ustring::compose(
        xml_fmt, parameters.get_str("source"), parameters.get_str("timestamp"),
        parameters.get_str("category"), parameters.get_str("level"),
        parameters.get_str("theme"), xml_escape(msg));
}

void LogFileEntry::build_from_hash(const std::map<std::string, Glib::ustring>& parameters)
{
    Glib::ustring msg = parameters.at("message");

    if (msg.size() > MAX_LOG_ENTRY_SIZE)
        msg.erase(MAX_LOG_ENTRY_SIZE, msg.size() - MAX_LOG_ENTRY_SIZE);

    msg = UstringUtils::strip_newlines_and_tabs(msg);

    _entry = Glib::ustring::compose(
        xml_fmt, parameters.at("source"), parameters.at("timestamp"),
        parameters.at("category"), parameters.at("level"), parameters.at("theme"),
        xml_escape(msg));
}
