#include <regex>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "conf_handler.h"
#include "file_handler.h"
#include "file_service.h"
#include "utils.h"

#include "watchdog.h"

Watchdog::Watchdog (const std::string& resource, const std::string& input, bool activate)
    : Glib::Object()
    , _activated{activate}
    , _in_progress{false}
    , _resource{resource}
    , _input_folder{input}
{}

Watchdog::InputProcessorList Watchdog::get_input_processors() const
{
    auto conf_handler = ConfHandler::get_instance();
    return conf_handler->getInputProcessorsByResource(_resource);
}

std::string Watchdog::build_regex_pattern() const
{
    auto conf = ConfHandler::get_instance ();
    // pattern: (?:instrument_code|serial_number)-X_X.ext
    std::stringstream ss;

    ss << "(" << conf->getParameter ("instrumentCode") << "|" << conf->getParameter ("serialnumber") << ")-";
    ss << "\\w+(_\\w+\\.(xml|XML))";

    return ss.str();
}

Watchdog::LanFolderList Watchdog::create_file_list(const std::string& folder) const
{
    LanFolderList result;

    auto files = FileHandler::list_directory(folder);

    for (auto&& file : files) {
        std::regex re(build_regex_pattern());
        std::smatch match;

        if (!std::regex_match(file, match, re))
            continue;

        result.emplace_back(file, folder + "/" + file, match[3], match[2], match[1]);
    }

    return result;
}

void Watchdog::sort_file_list(LanFolderList& list) const
{
    // lex compare
    std::sort(list.begin(), list.end(),
              [] (const LanFolderEntry& e1, const LanFolderEntry& e2) -> bool
              {
                  return std::lexicographical_compare(e1.path.begin(), e1.path.end(),
                                                      e2.path.begin(), e2.path.end());
              });
}

std::string Watchdog::get_log_file_name(const LanFolderEntry& entry) const
{
    auto conf = ConfHandler::get_instance ();
    // 1a/1b
    if (entry.id == conf->getParameter ("instrumentCode") ) {
        std::stringstream ss;
        ss << entry.file_name << "_" << conf->getParameter ("serialnumber") << ".log";
        return ss.str();
    } else {
        // 1c/1d
        std::string copy = entry.file_name;
        auto i = copy.find(".xml");
        if (i != std::string::npos)
            copy.replace(i, 4, ".log");
        i = copy.find(".XML");
        if (i != std::string::npos)
            copy.replace(i, 4, ".LOG");
        return copy;
    }
}

std::string Watchdog::get_log_folder() const
{
    auto conf_handler   = ConfHandler::get_instance();
    auto usb_log_folder = conf_handler->getFolder(this->get_file_destination(), "log");

    if (usb_log_folder.size() != 1)
        EXCEPTION("Couldn't determine log folder for file destination " <<
                  this->get_file_destination());

    return usb_log_folder[0].path();
}

void Watchdog::forward_entry_to_zix(const LanFolderEntry& entry)
{
    auto file_service = FileService::get_instance();

    std::ifstream ifs(entry.path);
    if (!ifs.good()) {
        PRINT_ERROR("Failed to open file " << entry.path);
        return;
    }

    std::string content, line;
    while (std::getline(ifs, line))
    {
        content += line;
        content += "\n";
    }

    if (ifs.bad()) {
        PRINT_ERROR("Failed to read line from file " << entry.path);
        return;
    }

    // send xml content to ZIX queue
    try {
        auto out_path = this->get_root_folder() + "/" + get_log_folder() + "/" +
            get_log_file_name(entry);

        file_service->new_file_content.emit(this->get_zix_interface(),
                                            entry.path, content, out_path);
    } catch (const std::exception& ex) {
        PRINT_ERROR(ex.what());
        return;
    }

    // move/copy immediatley after reading
    move_or_copy_xml_to_log_dir(entry);
}

void Watchdog::forward_entry_to_ip(const LanFolderEntry& entry, const InputProcessor& ip)
{
    _ip_proc = ProcessRequest::create(
        { ip.code(), "--file", entry.path }, ProcessRequest::DEFAULT_TIMEOUT);
    _ip_proc->finished.connect(
        sigc::mem_fun(*this, &Watchdog::on_ip_proc_ready));
    _ip_proc->start_process();
}

void Watchdog::on_ip_proc_ready(const Glib::RefPtr<ProcessResult>& result)
{
    if (!result->success()) {
        // at least, we tried...
        PRINT_ERROR("Inputprocessor script failed to execute: " << result->error_reason());
    }

    next();
}

void Watchdog::move_or_copy_xml_to_log_dir(const LanFolderEntry& entry)
{
    if (!FileHandler::file_exists(entry.path))
        return;

    try {
        // lower case xml
        const auto& path = entry.file_name;
        auto i = path.find(".xml");

        if (i != std::string::npos) {
            FileHandler::move_file(entry.path, this->get_root_folder() + "/" +
                                   get_log_folder() + "/" + entry.file_name);

            return;
        }

        // upper case xml
        i = path.find(".XML");
        if (i != std::string::npos) {
            FileHandler::copy_file(entry.path, this->get_root_folder() + "/" +
                                   get_log_folder() + "/" + entry.file_name);
        }
    } catch (const std::exception& ex) {
        PRINT_ERROR(ex.what());
    }
}

void Watchdog::next()
{
    // "small" xml files will be moved to log directory
    // "big" xml files will be copied to log directory
    move_or_copy_xml_to_log_dir(*_folder_it);

    // next?
    if (++_folder_it != _folder_list.cend()) {
        process_entry(*_folder_it);
        return;
    }

    // done
    _in_progress = false;
}

void Watchdog::process_entry(const LanFolderEntry& entry)
{
    PRINT_DEBUG ("Watchdog::process_entry seeing entry.pattern=" << entry.pattern);

    // batch file
    if (entry.pattern == "_zix.xml" ||
        entry.pattern == "_zix.XML") {
        forward_entry_to_zix(entry);
        next();
        return;
    }

    // input processor
    auto ips = get_input_processors();
    for (auto&& ip : ips) {
        // There should be only one pattern that matches
        if (entry.pattern == ip.pattern()) {
            forward_entry_to_ip(entry, ip);
            return;
        }
    }

    // nothing todo with this file -> next
    next();
}
