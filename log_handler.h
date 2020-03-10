#ifndef _LOG_HANDLER_H_
#define _LOG_HANDLER_H_

#include <string>
#include <cstddef>
#include <vector>
#include <map>

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <giomm/file.h>
#include <giomm/fileoutputstream.h>

#include "log_file_entry.h"
#include "xml_parameter_list.h"
#include "xml_result.h"

/**
 * ZIX-Logging Handler.
 *
 * This class provides low-level access to the ZIX logfile (/var/log/zix.log).
 *
 * This is implemented as singleton, so that there's only one class which has
 * a file handle to the ZIX log file.
 *
 * Logging is done by passing the xml parameters to the log() function.
 *
 */
class LogHandler : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<LogHandler>;
    using LogBuf = std::vector<LogFileEntry>;
    using ParMap = std::map<std::string, Glib::ustring>;

    /**
     * Log-Levels which can be used.
     */
    enum class LogLevel {
        FATAL_ERROR = 4,
        ERROR = 3,
        WARNING = 2,
        INFO = 1,
        DEBUG = 0,
        INVALID = -1,
    };

    /**
     * Usage like this:
     *   try {
     *     auto logger = LogHandler->get_instance();
     *     logger->log(xml_parameters);
     *   } catch (...) { }
     */
    static inline RefPtr get_instance()
    {
        if (!instance)
            instance = RefPtr(new LogHandler());
        return instance;
    }

    /**
     * This method performs the log request.
     *
     * Throws exception if something goes wrong.
     *
     * @param params xml parameters described by ZIX spec.
     */
    void log(const XmlParameterList& params);

    /**
     * This method performs the log request.
     *
     * Throws exception if something goes wrong.
     *
     * @param params xml parameters described by ZIX spec.
     */
    void log(const ParMap& params);

    /**
     * Creates log entry with log source internal
     *
     * Throws exception if something goes wrong.
     *
     * @param level     log level as string
     * @param theme     log theme
     * @param message   log message
     */
    void log_internal(const std::string& level, const std::string& theme, const std::string& message);

    /**
     * Flushes all buffered log entries to the log file.
     *
     * Throws an exception if something goes wrong.
     */
    void flush();

    /**
     * This function can be used for configuring the buffer size at runtime.
     *
     * @param new_buffer_size the new buffer size
     */
    void set_buffer_size(LogBuf::size_type new_buffer_size);

    /**
     * Gets the currently configured buffer size for the internal log message
     * buffer.
     *
     * @return buffer size
     */
    LogBuf::size_type get_buffer_size() const noexcept;

    /**
     * This function sets the current log level.
     *
     * @param new_level the new log level
     */
    void set_log_level(LogLevel new_level) noexcept;

    /**
     * Returns the current log level.
     *
     * @return current log level
     */
    LogLevel get_log_level() const noexcept;

    /**
     * Truncates the current log file to the given number of entries.
     *
     * Creates new file if something went wrong.
     *
     * @param numberOfLogKeep number of remaining enties of log file
     */
    void log_truncate(int numberOfLogKeep);

    /**
     * Gets the path for the use log file.
     *
     * @return path to log file
     */
    const std::string& get_log_file() const noexcept;

private:
    Glib::RefPtr<Gio::File> _file;
    Glib::RefPtr<Gio::FileOutputStream> _stream;
    LogBuf _buffer;
    std::size_t _idx;
    std::size_t _msgcnt;
    std::string _log_file;
    LogLevel _current_log_level;

    static RefPtr instance;

    LogHandler();

    bool log_common(const std::string& level_str, const LogFileEntry& entry);
    void log(const Glib::ustring& message);

    LogLevel to_log_level(const std::string& level) const;
    std::string to_log_string(LogHandler::LogLevel level) const;

    /**
     * React to loglevel and logbuffer size changes.
     *
     * @param parId parameter id
     * @param value the new value
     */
    void on_config_changed_announce (const Glib::ustring& parId, const Glib::ustring& value, int &handlerMask);
    void on_config_changed (const int handlerMask);

    /**
     * Adds the log entry into the ring buffer.
     *
     * @param entry log entry
     */
    void insert_into_buffer(const LogFileEntry& entry);
};

#endif /* _LOG_HANDLER_H_ */
