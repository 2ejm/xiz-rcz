#ifndef _FILE_HANDLER_H_
#define _FILE_HANDLER_H_

#include <string>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>

#include <giomm/unixoutputstream.h>

/**
 * This class provides static methods for handling filesystem requests.
 */
class FileHandler
{
public:
    /**
     * List SIC directory (without "." and "..").
     *
     * @param path directoy
     *
     * @return directory listing
     */
    static std::vector<std::string> list_directory(const std::string& path);

    /**
     * List SIC directory files
     *
     * @param path directoy
     *
     * @return directory files listing
     */
    static std::vector<std::string> list_directory_files(const std::string& path);

    /**
     * Creates a directory. Works like `mkdir -p`.
     *
     * @param path directoy
     */
    static void create_directory(const std::string& path);

    /**
     * Removes a directory like `rm -rf`.
     *
     * @param path directoy
     */
    static void remove_directory(const std::string& path);

    /**
     * Get file contents. Do not use for huge files.
     *
     * @param path file
     *
     * @return bytes as vector
     */
    static std::string get_file(const std::string& path);

    /**
     * Writes file to disk.
     *
     * @param path    file
     * @param content file content
     */
    static void set_file(const std::string& path, const std::string& content);

    /**
     * Deletes a given file
     *
     * @param path file to delete.
     */
    static void del_file(const std::string& path);

    /**
     * Moves files specified by path1 to path2.
     *
     * @param path1 input file path
     * @param path2 output file path
     */
    static void move_file(const std::string& path1, const std::string& path2);

    /**
     * Copies file specified by path1 to path2.
     *
     * @param path1 input file path
     * @param path2 output file path
     */
    static void copy_file(const std::string& path1, const std::string& path2);

    /**
     * Encode Base64.
     *
     * @param input input data
     *
     * @return encoded data
     */
    static std::string base64_encode(const std::string& input);

    /**
     * Decode Base64.
     *
     * @param input data
     *
     * @return decoded data
     */
    static std::string base64_decode(const std::string& input);

    /**
     * This functions checks whether a given file exists or is accessible.
     *
     * @param name path of the file
     *
     * @return true if accessible, else false
     */
    static bool file_exists(const std::string& name);

    /**
     * This functions checks whether a given directory exists or is accessible.
     *
     * @param name path of the file
     *
     * @return true if accessible, else false
     */
    static bool directory_exists(const std::string& name);

    /**
     * This functions checks whether a given path is a socket.
     *
     * @param name path of the file
     *
     * @return true if socket and accessible, else false
     */
    static bool socket_exists(const std::string& name);

    /**
     * Gets basename. This functions uses Gio::File, instead of basename(3).
     *
     * @param file_name file
     *
     * @return basename
     */
    static std::string basename(const std::string& file_name);

    /**
     * Gets dirname. This functions uses Glib, instead of dirname(3).
     *
     * @param file_name file
     *
     * @return dirname
     */
    static std::string dirname(const std::string& file_name);

    /**
     * Creates a tempory file and return an output stream to it.
     *
     * @param name name of the new file (output parameter!)
     *
     * @return output stream
     */
    static Glib::RefPtr<Gio::UnixOutputStream> get_temp_file_write(std::string& name);

    /**
     * Change owner and group of a file.
     *
     * @param path   path to file
     * @param owner  new owner
     * @param group  new group
     */
    static void chown(const std::string& path, const std::string& owner, const std::string& group);

    /**
     * Change mode of a file.
     *
     * @param path  path to file
     * @param mode  new mode
     */
    static void chmod(const std::string& path, mode_t mode);

    /**
     * Get user id for user name.
     *
     * @param user user name
     *
     * @return user id
     */
    static uid_t get_user_id(const std::string& user);

    /**
     * Get group id for group name.
     *
     * @param group group name
     *
     * @return group id
     */
    static gid_t get_groupd_id(const std::string& group);

    /**
     * Truncates the logfile to N lines; where N \in number_of_log_keep.
     *
     * @param log_file
     * @param number_of_log_keep
     */
    static void log_truncate(const std::string& log_file, int number_of_log_keep);

private:
    FileHandler()
    {}
};

#endif /* _FILE_HANDLER_H_ */
