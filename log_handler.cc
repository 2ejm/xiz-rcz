#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "conf_handler.h"
#include "file_handler.h"
#include "time_utilities.h"
#include "utils.h"

#include "log_handler.h"

LogHandler::RefPtr LogHandler::instance;

LogHandler::LogHandler() :
    _idx{0}, _msgcnt{0}
{
    #ifdef GLOBAL_INSTALLATION
        _log_file = "/var/log/zix.log";
    #else
        _log_file = "zix.log";
    #endif

    // setup configuration values
    auto conf_handler = ConfHandler::get_instance();
    auto buffer_size  = conf_handler->getLogBufferSize();
    auto level        = conf_handler->getLogLevel();

    if (buffer_size == 0) {
        PRINT_ERROR("Failed to get log buffer size from zixconf.xml. Using a meaningful default.");
        _buffer.resize(1 << 6);
    } else {
        _buffer.resize(buffer_size);
    }

    if (level.empty()) {
        PRINT_ERROR("Failed to get log level from zixconf.xml. Using a meaningful default.");
        _current_log_level = LogLevel::INFO;
    } else {
        _current_log_level = to_log_level(level);
    }

    // open log file
    try {
        _file   = Gio::File::create_for_path(_log_file);
        _stream = _file->append_to();
    } catch (const Glib::Error& ex) {
        EXCEPTION("Failed to open/create ZIX log file " << _log_file << ": " << ex.what());
    }

    // connect to config changed signal
    conf_handler->confChangeAnnounce.connect(
        sigc::mem_fun(*this, &LogHandler::on_config_changed_announce));
    conf_handler->confChanged.connect(
        sigc::mem_fun(*this, &LogHandler::on_config_changed));

    // Now we can actually use the libzix logging
    setLogMechanism(ELogMechanismLibzix);
}

void LogHandler::insert_into_buffer(const LogFileEntry& entry)
{
    const auto buffer_size = _buffer.size();
    _buffer[_idx] = entry;
    _idx = (_idx + 1) % buffer_size;
    ++_msgcnt;
}

bool LogHandler::log_common(const std::string& level_str, const LogFileEntry& entry)
{
    LogLevel level = to_log_level(level_str);

    // don't log if log level is too high
    if (level < _current_log_level) {
        insert_into_buffer(entry);
        return true;
    }

    // on error/fatal error -> flush the buffer into the log file
    if (level == LogLevel::ERROR || level == LogLevel::FATAL_ERROR) {
        insert_into_buffer(entry);
        flush();
        return true;
    }

    return false;
}

void LogHandler::log(const XmlParameterList& params)
{
    LogFileEntry entry(params);
    auto handled = log_common(params.get_str("level"), entry);

    if (!handled)
        log(entry.entry());
}

void LogHandler::log(const ParMap& params)
{
    LogFileEntry entry(params);
    auto handled = log_common(params.at("level"), entry);

    if (!handled)
        log(entry.entry());
}

void LogHandler::log_internal(const std::string& level, const std::string& theme,
                              const std::string& message)
{
    ParMap parameters;

    parameters["source"]    = "SIC";
    parameters["timestamp"] = TimeUtilities::get_timestamp();
    parameters["category"]  = "System";
    parameters["level"]     = level;
    parameters["theme"]     = theme;
    parameters["message"]   = message;

    log(parameters);
}

void LogHandler::log(const Glib::ustring& message)
{
    gsize bytes_written;
    auto ret = _stream->write_all(message, bytes_written);

    if (!ret)
        EXCEPTION("Failed to write message to log file. Bytes written: "
                  << bytes_written);
}

void LogHandler::flush()
{
    const auto        buffer_size = _buffer.size();
    const std::size_t entries     = std::min(_msgcnt, buffer_size);
    const std::size_t start       = _msgcnt >= buffer_size ? _idx : 0;

    for (std::size_t i = start; i < start + entries; ++i)
        log(_buffer[i % entries].entry());

    _idx    = 0;
    _msgcnt = 0;
}

LogHandler::LogLevel LogHandler::to_log_level(const std::string& level) const
{
    if (level == "Fatal Error")
        return LogLevel::FATAL_ERROR;
    else if (level == "Error")
        return LogLevel::ERROR;
    else if (level == "Warning")
        return LogLevel::WARNING;
    else if (level == "Info")
        return LogLevel::INFO;
    else if (level == "Debug")
        return LogLevel::DEBUG;

    EXCEPTION("Unknown log level '" << level << "' specified");
}

std::string LogHandler::to_log_string(LogHandler::LogLevel level) const
{
    switch (level) {
    case LogLevel::FATAL_ERROR: return "Fatal Error";
    case LogLevel::ERROR:       return "Error";
    case LogLevel::WARNING:     return "Warning";
    case LogLevel::INFO:        return "Info";
    case LogLevel::DEBUG:       return "Debug";
    default:
        EXCEPTION("Unknown log level '" << static_cast<int>(level) << "' specified");
    }

    return "Unknown";
}

void LogHandler::log_truncate(int numberOfLogKeep)
{
    _stream->close();

    FileHandler::log_truncate(_log_file, numberOfLogKeep);

    /* Gets an output stream for appendig data to the file. If
     * the file doesn't exist it is created.
     */
    _stream = _file->append_to();
}

void LogHandler::set_log_level(LogLevel new_level) noexcept
{
    _current_log_level = new_level;
}

LogHandler::LogLevel LogHandler::get_log_level() const noexcept
{
    return _current_log_level;
}

const std::string& LogHandler::get_log_file() const noexcept
{
    return _log_file;
}

void LogHandler::set_buffer_size(LogBuf::size_type new_buffer_size)
{
    _buffer.resize(new_buffer_size);
}

LogHandler::LogBuf::size_type LogHandler::get_buffer_size() const noexcept
{
    return _buffer.size();
}

void LogHandler::on_config_changed_announce (const Glib::ustring& par_id, const Glib::ustring& value, int &handlerMask)
{
    (void) value;

    if ( (par_id == "logLevel")
      || (par_id == "logBufferSize")
      || (par_id == "all" ) )
	handlerMask |= HANDLER_MASK_LOG_HANDLER;
}

void LogHandler::on_config_changed (const int handlerMask)
{
    if (handlerMask & HANDLER_MASK_LOG_HANDLER) {
	auto conf_handler = ConfHandler::get_instance();
	auto buffer_size  = conf_handler->getLogBufferSize();
	auto level        = conf_handler->getLogLevel();

	/* check value for validity
	 * 0 means means that the value is not
	 * configured properly
	 */
	if (buffer_size == 0) {
	    _buffer.resize(1 << 6);
	    _idx    = 0;
	    _msgcnt = 0;
	    PRINT_ERROR("Failed to get log buffer size from zixconf.xml. Using a meaningful default.");
	} else {
	    /* check, whether this resize is really necessary,
	     * because we loose all values in the ringbuffer
	     */
	    if (_buffer.size () != buffer_size) {
		_buffer.resize(buffer_size);
		_idx    = 0;
		_msgcnt = 0;
	    }
	}

	if (level.empty()) {
	    PRINT_ERROR("Failed to get log level from zixconf.xml. Using a meaningful default.");
	    _current_log_level = LogLevel::INFO;
	} else {
	    _current_log_level = to_log_level(level);
	}
    }
}
