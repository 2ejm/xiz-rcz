#include <regex>
#include <algorithm>

#include "post_processing_builder.h"

#include "file_handler.h"
#include "conf_handler.h"
#include "utils.h"

std::string PostProcessingBuilder::get_input_extension(const Glib::ustring& code) const
{
    std::string script = FileHandler::basename(code);
    std::string input_ext = script.substr(0, 3);

    if (input_ext != "xml" && input_ext != "bmp")
        EXCEPTION("Couldn't determine input extension from post processor.");

    return input_ext;
}

PostProcessingBuilder::PostProcessingQueue PostProcessingBuilder::build_queue_without_dest(
    const Glib::ustring& iid, const Glib::ustring& format)
{
    PostProcessingQueue result;

    auto conf_handler = ConfHandler::get_instance();

    // each folder should have a format -> get postprocessor for it
    auto pp = conf_handler->getSpecificPostProcessor(format);

    if (pp.code().empty())
        EXCEPTION("Failed to get post processor for format " << format);

    // a post processor can generate multiple files, we have to find out which ones
    auto generated_files = generated_files_by_pp(pp, iid, format);

    for (auto&& generated_file : generated_files) {
        PostProcessingEntry e;

        e.format         = format;
        e.post_processor = pp.code();
        e.output_file    = "";
        e.generated_file = generated_file;
        e.iid            = iid;

        PRINT_DEBUG("Entry: " << generated_file);

        result.push_back(e);
    }

    return result;
}

PostProcessingBuilder::PostProcessingQueue PostProcessingBuilder::build_queue(
    const Glib::ustring& iid, const FileDestination& dest,
    const Glib::ustring& format, const Glib::ustring& output)
{
    if (dest.is_file_based_dest())
        return build_queue_file_dest(iid, dest, format, output);
    else
        return build_queue_other_dest(iid, dest, format);
}

PostProcessingBuilder::PostProcessingQueue PostProcessingBuilder::build_queue_file_dest(
    const Glib::ustring& iid, const FileDestination& dest,
    const Glib::ustring& format, const Glib::ustring& output)
{
    PostProcessingQueue result;

    lHighDebug ("PostProcessingBuilder: iid=%s, dest=%s format=%s output=%s\n", iid.c_str(), dest.to_string().c_str(), format.c_str(), output.c_str());

    auto conf_handler = ConfHandler::get_instance();
    auto root_folder  = conf_handler->getRootFolder(dest);
    auto serial       = conf_handler->getParameter("serialnumber");

    if (serial.empty())
        EXCEPTION("Couldn't get serial number via config");

    // get folder for destination and type (-> measurement)
    auto folder = conf_handler->getFolder(dest.to_string(), "measurement");

    if (folder.empty())
        EXCEPTION("No folders for output.");

    for (auto&& entry : folder) {
        PRINT_DEBUG("Entry: format=" << entry.format() << "; id=" << entry.id());
    }

    // remove folders where output id doesn't match
    if (!output.empty() && !folder.empty() )
        folder.erase(
            std::remove_if(folder.begin(), folder.end(),
                       [&output] (const Folder& f)
                       {
                           return output != f.id();
                       }), folder.end());

    // remove folders where format doesn't match
    if (!format.empty() && !folder.empty() )
        folder.erase(
            std::remove_if(folder.begin(), folder.end(),
                       [&format] (const Folder& f)
                       {
                           return format != f.format();
                       }), folder.end());

    if (folder.empty())
        EXCEPTION("Format or Output id is unknown or not configured. output=" << output << "; format=" << format);

    // each folder should have a format -> get postprocessor for it
    for (auto&& entry : folder) {
        if (entry.format().empty()) {
            PRINT_DEBUG("No format found...");
            continue;
        }

        auto pp = conf_handler->getSpecificPostProcessor(dest.to_string(), entry.format());

        if (pp.code().empty())
            EXCEPTION("Failed to get post processor for folder " << entry.path());

        // a post processor can generate multiple files, we have to find out which ones
        auto generated_files = generated_files_by_pp(pp, iid, entry.format());

        for (auto&& generated_file : generated_files) {
            PostProcessingEntry e;

            e.format         = entry.format();
            e.post_processor = pp.code();
            e.generated_file = generated_file;
            e.iid            = iid;
            e.output_file    = root_folder + "/" + entry.path() + "/" + serial +
                "__" + FileHandler::basename(generated_file);

            PRINT_DEBUG("Entry: " << generated_file << " --> " << e.output_file);

            result.push_back(e);
        }
    }

    return result;
}

PostProcessingBuilder::PostProcessingQueue PostProcessingBuilder::build_queue_other_dest(
    const Glib::ustring& iid, const FileDestination& dest, const Glib::ustring& format)
{
    PostProcessingQueue result;

    auto conf_handler = ConfHandler::get_instance();

    if (format.empty())
        EXCEPTION("Format has to be given for destination: " << dest.to_string());

    // each folder should have a format -> get postprocessor for it
    auto pp = conf_handler->getSpecificPostProcessor(dest.to_string(), format);

    if (pp.code().empty())
        EXCEPTION("Failed to get post processor for destination " << dest.to_string());

    // a post processor can generate multiple files, we have to find out which ones
    auto generated_files = generated_files_by_pp(pp, iid, format);

    for (auto&& generated_file : generated_files) {
        PostProcessingEntry e;

        e.format         = format;
        e.post_processor = pp.code();
        e.output_file    = "";
        e.generated_file = generated_file;
        e.iid            = iid;

        PRINT_DEBUG("Entry: " << generated_file);

        result.push_back(e);
    }

    return result;
}

std::vector<std::string> PostProcessingBuilder::generated_files_by_pp(
    const PostProcessor& pp, const Glib::ustring& iid, const Glib::ustring& format_name) const
{
    std::vector<std::string> result;
    auto input_ext = get_input_extension(pp.code());

    // find referenced measurement files
    auto        conf_handler = ConfHandler::get_instance();
    std::string dir          = conf_handler->getDirectory("measurements");
    if (dir.empty())
        EXCEPTION("Couldn't get measurement directory via config");
    dir += "/";
    dir += iid;
    auto files = FileHandler::list_directory(dir);

    // find measurement files
    for (auto&& file : files) {
        std::string iid_pattern;

        //
        // Measurement files follow this pattern:
        //  -> XML: <iid>.xml
        //  -> BMP: <iid><tag>.bmp
        //
        if (input_ext == "xml")
            iid_pattern = iid + "\\." + input_ext;
        else
            iid_pattern = iid + "(.*?)\\." + input_ext;

        std::regex re_iid(iid_pattern);
        std::smatch match;

        if (!std::regex_match(file, match, re_iid))
            continue;

        // generate output file for it
        auto format = pp.get_format_by_name(format_name);
        std::string tag = match.size() == 2 ? match.str(1) : "";
        std::string generated_file;
        generated_file = dir + "/" + iid +
            (!tag.empty() ? tag : format.tag().raw()) + "." + format.ext();

        result.push_back(generated_file);
    }

    if (!result.size())
        EXCEPTION("No measurement files found");

    return result;
}
