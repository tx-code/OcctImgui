/**
 * @file Logger.h
 * @brief Defines the Logger class that provides hierarchical logging functionality.
 */
#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <string>

namespace Utils
{

// Forward declaration
class LoggerManager;

/**
 * @class Logger
 * @brief Provides hierarchical logging functionality.
 *
 * This class provides hierarchical logging with context tracking and call chain tracking.
 */
class Logger: public std::enable_shared_from_this<Logger>
{
public:
    /**
     * @brief Get a logger for the specified module
     *
     * @param module Module name
     * @return Shared pointer to the logger
     */
    static std::shared_ptr<Logger> getLogger(const std::string& module);

    /**
     * @brief Initialize the logging system
     *
     * @param logDirPath Path to the log directory
     * @return True if initialization was successful, false otherwise
     */
    static bool initialize(const std::string& logDirPath = "logs");

    /**
     * @brief Constructor
     *
     * @param module Module name
     */
    Logger(const std::string& module);

    /**
     * @brief Create a child logger
     *
     * @param subModule Sub-module name
     * @return Shared pointer to the child logger
     */
    std::shared_ptr<Logger> createChild(const std::string& subModule);

    /**
     * @brief Set the context ID
     *
     * @param contextId Context ID
     */
    void setContextId(const std::string& contextId);

    /**
     * @brief Get the full log prefix
     *
     * @return Log prefix
     */
    std::string getPrefix() const;

    // Log level methods
    template<typename... Args>
    void trace(const std::string& fmt, const Args&... args)
    {
        auto logger = spdlog::get(myModule);
        if (logger) {
            logger->trace(getPrefix() + " " + fmt, args...);
        }
        else {
            spdlog::trace(getPrefix() + " " + fmt, args...);
        }
    }

    template<typename... Args>
    void debug(const std::string& fmt, const Args&... args)
    {
        auto logger = spdlog::get(myModule);
        if (logger) {
            logger->debug(getPrefix() + " " + fmt, args...);
        }
        else {
            spdlog::debug(getPrefix() + " " + fmt, args...);
        }
    }

    template<typename... Args>
    void info(const std::string& fmt, const Args&... args)
    {
        auto logger = spdlog::get(myModule);
        if (logger) {
            logger->info(getPrefix() + " " + fmt, args...);
        }
        else {
            spdlog::info(getPrefix() + " " + fmt, args...);
        }
    }

    template<typename... Args>
    void warn(const std::string& fmt, const Args&... args)
    {
        auto logger = spdlog::get(myModule);
        if (logger) {
            logger->warn(getPrefix() + " " + fmt, args...);
        }
        else {
            spdlog::warn(getPrefix() + " " + fmt, args...);
        }
    }

    template<typename... Args>
    void error(const std::string& fmt, const Args&... args)
    {
        auto logger = spdlog::get(myModule);
        if (logger) {
            logger->error(getPrefix() + " " + fmt, args...);
        }
        else {
            spdlog::error(getPrefix() + " " + fmt, args...);
        }
    }

    template<typename... Args>
    void critical(const std::string& fmt, const Args&... args)
    {
        auto logger = spdlog::get(myModule);
        if (logger) {
            logger->critical(getPrefix() + " " + fmt, args...);
        }
        else {
            spdlog::critical(getPrefix() + " " + fmt, args...);
        }
    }

    /**
     * @class ScopedLogger
     * @brief Logs function entry and exit
     *
     * This class logs function entry when constructed and function exit when destructed.
     */
    class ScopedLogger
    {
    public:
        /**
         * @brief Constructor
         *
         * @param logger Logger to use
         * @param functionName Function name
         */
        ScopedLogger(std::shared_ptr<Logger> logger, const std::string& functionName);

        /**
         * @brief Destructor
         */
        ~ScopedLogger();

    private:
        std::shared_ptr<Logger> myLogger;
        std::string myFunctionName;
    };

    /**
     * @brief Create a function scope logger
     *
     * @param functionName Function name
     * @return ScopedLogger instance
     */
    ScopedLogger functionScope(const std::string& functionName);

private:
    std::string myModule;
    std::string myContextId;
};

// Convenience macro for creating function scope loggers
#define LOG_FUNCTION_SCOPE(logger, function) auto scopedLogger = logger->functionScope(function)

// Convenience macros for getting loggers with consistent naming
#define DECLARE_LOGGER(className)                                                                  \
    static std::shared_ptr<Utils::Logger>& get##className##Logger()                                \
    {                                                                                              \
        static std::shared_ptr<Utils::Logger> logger = Utils::Logger::getLogger(#className);       \
        return logger;                                                                             \
    }

#define DECLARE_STATIC_LOGGER(moduleName)                                                          \
    static std::shared_ptr<Utils::Logger>& get##moduleName##Logger()                               \
    {                                                                                              \
        static std::shared_ptr<Utils::Logger> logger = Utils::Logger::getLogger(moduleName);       \
        return logger;                                                                             \
    }

#define DECLARE_NAMESPACE_LOGGER(namespaceName, className)                                         \
    static std::shared_ptr<Utils::Logger>& get##namespaceName####className##Logger()               \
    {                                                                                              \
        static std::shared_ptr<Utils::Logger> logger =                                             \
            Utils::Logger::getLogger(#namespaceName "." #className);                               \
        return logger;                                                                             \
    }

}  // namespace Utils