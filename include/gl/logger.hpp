#ifndef GL_LOGGER_HPP
#define GL_LOGGER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <mutex>
#include <vector>
#include <chrono>
#include <source_location>
#include <iostream>

namespace gl {

    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Fatal,
        Off
    };

    class Logger {
    public:
        static Logger& instance() {
            static Logger instance;
            return instance;
        }

        void setLevel(LogLevel level) {
            currentLevel_ = level;
        }

        void setOutputFile(const std::string& filename) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (fileOutput_.is_open()) {
                fileOutput_.close();
            }
            fileOutput_.open(filename, std::ios::app);
        }

        template<typename... Args>
        void log(LogLevel level,
            const std::string& message,
            const std::source_location& location = std::source_location::current()) {
            if (level < currentLevel_) return;

            std::stringstream ss;
            // Add timestamp
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            ss << std::ctime(&time);
            ss.seekp(-1, ss.cur); // Remove trailing newline

            // Add level
            ss << " [" << levelToString(level) << "] ";

            // Add source info
            ss << location.file_name() << ":" << location.line() << " ";

            // Add message
            ss << message;

            std::string output = ss.str();

            std::lock_guard<std::mutex> lock(mutex_);

            // In debug mode, still output to console for severe events
            if (level >= LogLevel::Warning) {
                std::cerr << output << std::endl;
            }

            // Write to file if opened
            if (fileOutput_.is_open()) {
                fileOutput_ << output << std::endl;
                fileOutput_.flush();
            }

            // Store in memory buffer (circular buffer concept)
            logBuffer_.push_back(output);
            if (logBuffer_.size() > bufferSize_) {
                logBuffer_.erase(logBuffer_.begin());
            }
        }

        // Specific level logging methods
        template<typename... Args>
        void debug(const std::string& message, const std::source_location& location = std::source_location::current()) {
            log(LogLevel::Debug, message, location);
        }

        template<typename... Args>
        void info(const std::string& message, const std::source_location& location = std::source_location::current()) {
            log(LogLevel::Info, message, location);
        }

        template<typename... Args>
        void warning(const std::string& message, const std::source_location& location = std::source_location::current()) {
            log(LogLevel::Warning, message, location);
        }

        template<typename... Args>
        void error(const std::string& message, const std::source_location& location = std::source_location::current()) {
            log(LogLevel::Error, message, location);
        }

        std::vector<std::string> getRecentLogs(size_t count = 0) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (count == 0 || count > logBuffer_.size()) {
                return logBuffer_;
            }
            return std::vector<std::string>(logBuffer_.end() - count, logBuffer_.end());
        }

    private:
        Logger() : currentLevel_(LogLevel::Info), bufferSize_(1000) {}
        ~Logger() {
            if (fileOutput_.is_open()) {
                fileOutput_.close();
            }
        }

        std::string levelToString(LogLevel level) {
            switch (level) {
            case LogLevel::Trace: return "TRACE";
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info: return "INFO";
            case LogLevel::Warning: return "WARN";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Fatal: return "FATAL";
            default: return "UNKNOWN";
            }
        }

        LogLevel currentLevel_;
        std::mutex mutex_;
        std::ofstream fileOutput_;
        std::vector<std::string> logBuffer_;
        size_t bufferSize_;
    };

    // Global shorthand methods
    inline void setLogLevel(LogLevel level) {
        Logger::instance().setLevel(level);
    }

    inline void setLogFile(const std::string& filename) {
        Logger::instance().setOutputFile(filename);
    }

    template<typename... Args>
    inline void logDebug(const std::string& message, const std::source_location& loc = std::source_location::current()) {
        Logger::instance().debug(message, loc);
    }

    template<typename... Args>
    inline void logInfo(const std::string& message, const std::source_location& loc = std::source_location::current()) {
        Logger::instance().info(message, loc);
    }

    template<typename... Args>
    inline void logWarning(const std::string& message, const std::source_location& loc = std::source_location::current()) {
        Logger::instance().warning(message, loc);
    }

    template<typename... Args>
    inline void logError(const std::string& message, const std::source_location& loc = std::source_location::current()) {
        Logger::instance().error(message, loc);
    }

} // namespace gl

#endif // GL_LOGGER_HPP