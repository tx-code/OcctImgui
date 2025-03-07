/**
 * @file LoggerManager.h
 * @brief Defines the LoggerManager class that manages application logging.
 */
#pragma once

#include <memory>
#include <string>
#include <vector>

namespace spdlog
{
class logger;
}

namespace Utils
{

class Logger;

/**
 * @class LoggerManager
 * @brief Manages application logging configuration and initialization.
 *
 * This class is responsible for initializing and configuring the logging system,
 * creating logger instances, and managing log files.
 */
class LoggerManager
{
public:
    /**
     * @brief Get the singleton instance of LoggerManager
     *
     * @return Reference to the LoggerManager instance
     */
    static LoggerManager& getInstance();

    /**
     * @brief Initialize the logging system
     *
     * Creates log directory, configures sinks, and sets up default loggers.
     *
     * @param logDirPath Path to the directory where log files will be stored
     * @return True if initialization was successful, false otherwise
     */
    bool initialize(const std::string& logDirPath = "logs");

    /**
     * @brief Get a logger by name
     *
     * If the logger doesn't exist, it will be created.
     *
     * @param name The name of the logger
     * @return Shared pointer to the logger
     */
    std::shared_ptr<Logger> getLogger(const std::string& name);

    /**
     * @brief Get the root logger
     *
     * @return Shared pointer to the root logger
     */
    std::shared_ptr<Logger> getRootLogger();

    /**
     * @brief Get the session ID
     *
     * @return The current session ID
     */
    const std::string& getSessionId() const;

    /**
     * @brief Get the log file path
     *
     * @return The path to the current log file
     */
    const std::string& getLogFilePath() const;

private:
    /**
     * @brief Constructor (private for singleton)
     */
    LoggerManager();

    /**
     * @brief Destructor
     */
    ~LoggerManager();

    /**
     * @brief Create a timestamp-based session ID
     *
     * @return The generated session ID
     */
    std::string createSessionId();

    /**
     * @brief Create and register a logger
     *
     * @param name The name of the logger
     * @return Shared pointer to the created logger
     */
    std::shared_ptr<Logger> createLogger(const std::string& name);

    /**
     * @brief Create the log directory if it doesn't exist
     *
     * @param logDirPath Path to the log directory
     * @return True if the directory exists or was created successfully, false otherwise
     */
    bool createLogDirectory(const std::string& logDirPath);

    /** Flag indicating if the manager has been initialized */
    bool myIsInitialized;

    /** Session ID */
    std::string mySessionId;

    /** Log file path */
    std::string myLogFilePath;

    /** Console sink */
    std::shared_ptr<void> myConsoleSink;

    /** File sink */
    std::shared_ptr<void> myFileSink;

    /** Root logger */
    std::shared_ptr<Logger> myRootLogger;

    /** Map of logger names to logger instances */
    std::vector<std::pair<std::string, std::shared_ptr<Logger>>> myLoggers;
};

}  // namespace Utils