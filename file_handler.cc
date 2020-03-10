#include <stdexcept>
#include <cstring>

#include <glibmm/fileutils.h>
#include <glibmm/ustring.h>
#include <glibmm/miscutils.h>
#include <glibmm/base64.h>
#include <giomm/file.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#include "file_handler.h"

#include "utils.h"

std::vector<std::string> FileHandler::list_directory(const std::string& path)
{
    try {
        Glib::Dir dir(path);
        return { dir.begin(), dir.end() };
    } catch (const Glib::Error& ex) {
        EXCEPTION(ex.what());
    }

    return { };
}

std::vector<std::string> FileHandler::list_directory_files(const std::string& path)
{
    std::vector<std::string> dir = FileHandler::list_directory(path);
    std::vector<std::string> files;

    for (auto&& name : dir) {
        auto filename = Glib::build_filename(path, name);
        if (!Glib::file_test(filename, Glib::FILE_TEST_IS_DIR))
            files.push_back(name);
    }

    return files;
}

void FileHandler::create_directory(const std::string& path)
{
    if (directory_exists(path))
        return;

    try {
        auto file = Gio::File::create_for_path(path);
        file->make_directory_with_parents();
    } catch (const Glib::Error& ex) {
        EXCEPTION("Failed to create directory " << path << ": " << ex.what());
    }
}

void FileHandler::remove_directory(const std::string& path)
{
    if (!directory_exists(path))
        return;

    auto dir = list_directory(path);
    for (auto&& file : dir) {
        auto entry = path + "/" + file;
        if (directory_exists(entry))
            remove_directory(entry);
        else
            del_file(entry);
    }
    del_file(path);
}

void FileHandler::del_file(const std::string& path)
{
    try {
        auto file = Gio::File::create_for_path(path);
        file->remove();
    } catch (const Glib::Error& ex) {
        EXCEPTION("Failed to delete file " << path << ": " << ex.what());
    }
}

std::string FileHandler::get_file(const std::string& path)
{
    std::string result;

    try {
        char *data;
        gsize length;
        std::string etag;

        auto file = Gio::File::create_for_path(path);
        file->load_contents(data, length, etag);

        result.insert(0, data, length);

        g_free(data);
    } catch (const Glib::Error& ex) {
        EXCEPTION("Failed to read file " << path << ": " << ex.what());
    }

    return result;
}

void FileHandler::set_file(const std::string& path, const std::string& content)
{
    try {
        std::string etag, etag_new;
        auto file = Gio::File::create_for_path(path);

        file->replace_contents(content.data(), content.size(), etag, etag_new,
                               false, Gio::FILE_CREATE_PRIVATE);
    } catch (const Glib::Error& ex) {
        EXCEPTION("Failed to write file " << path << ": " << ex.what());
    }
}

void FileHandler::move_file(const std::string& path1, const std::string& path2)
{
    try {
        auto file1 = Gio::File::create_for_path(path1);
        auto file2 = Gio::File::create_for_path(path2);

        file1->move(file2, Gio::FILE_COPY_OVERWRITE);
    } catch (const Glib::Error& ex) {
        EXCEPTION("Failed to move file " << path1 << " to " << path2 << ": " << ex.what());
    }
}

void FileHandler::copy_file(const std::string& path1, const std::string& path2)
{
    try {
        auto file1 = Gio::File::create_for_path(path1);
        auto file2 = Gio::File::create_for_path(path2);

        file1->copy(file2, Gio::FILE_COPY_OVERWRITE);
    } catch (const Glib::Error& ex) {
        EXCEPTION("Failed to copy file " << path1 << " to " << path2 << ": " << ex.what());
    }
}

std::string FileHandler::base64_encode(const std::string& input)
{
    return Glib::Base64::encode(input, false);
}

std::string FileHandler::base64_decode(const std::string& input)
{
    return Glib::Base64::decode(input);
}

bool FileHandler::file_exists(const std::string& name)
{
    struct stat sb;

    if (stat(name.c_str(), &sb))
        return false;
    return S_ISREG(sb.st_mode);
}

bool FileHandler::directory_exists(const std::string& name)
{
    struct stat sb;

    if (stat(name.c_str(), &sb))
        return false;
    return S_ISDIR(sb.st_mode);
}

bool FileHandler::socket_exists(const std::string& name)
{
    struct stat sb;

    if (stat(name.c_str(), &sb))
        return false;
    return S_ISSOCK(sb.st_mode);
}

std::string FileHandler::basename(const std::string& file_name)
{
    auto file = Gio::File::create_for_path(file_name);

    return file->get_basename();
}

std::string FileHandler::dirname(const std::string& file_name)
{
    return Glib::path_get_dirname(file_name);
}

Glib::RefPtr<Gio::UnixOutputStream> FileHandler::get_temp_file_write(std::string& name)
{
    int fd;

    try {
        fd = Glib::file_open_tmp(name);
    } catch (const Glib::Error& ex) {
        EXCEPTION("Failed to create temporary file: " << ex.what());
    }

    if (fd < 0)
        EXCEPTION("Failed to create temporary file");

    return Gio::UnixOutputStream::create(fd, true);
}

uid_t FileHandler::get_user_id(const std::string& user)
{
    auto *pwd = getpwnam(user.c_str());
    if (!pwd)
        EXCEPTION("Failed to get passwd information for user " << user << ": " <<
                  strerror(errno));

    return pwd->pw_uid;
}

gid_t FileHandler::get_groupd_id(const std::string& group)
{
    auto *g = getgrnam(group.c_str());
    if (!g)
        EXCEPTION("Failed to get group information for group " << group << ": " <<
                  strerror(errno));

    return g->gr_gid;
}

void FileHandler::chown(const std::string& path, const std::string& owner,
                        const std::string& group)
{
    auto ret = ::chown(path.c_str(), get_user_id(owner), get_groupd_id(group));
    if (ret)
        EXCEPTION("Failed to change owner/group of file " << path << ": " <<
                  strerror(errno));
}

void FileHandler::chmod(const std::string& path, mode_t mode)
{
    auto ret = ::chmod(path.c_str(), mode);

    if (ret)
        EXCEPTION("Failed to change permissions for file " << path << ": " <<
                  strerror(errno));
}

void FileHandler::log_truncate(const std::string& log_file, int number_of_log_keep)
{
    // save current log file
    std::string log_file_tmp = log_file + ".tmp";
    if (FileHandler::file_exists(log_file))
        FileHandler::move_file(log_file, log_file_tmp);

    std::string tail_cmd = "tail -q -n " + std::to_string(number_of_log_keep) + " "
                         + log_file_tmp + " > " + log_file;

    auto status = system(tail_cmd.c_str());

    if (status != 0) {
        PRINT_WARNING("Failed to truncate LogFile. Continue with new file, some log entries might be missing.");
        if (FileHandler::file_exists(log_file))
            FileHandler::del_file(log_file);
    }

    if (FileHandler::file_exists(log_file_tmp))
        FileHandler::del_file(log_file_tmp);
}
