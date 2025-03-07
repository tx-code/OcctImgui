#include "LoggerManager.h"
#include "Logger.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Utils
{

LoggerManager& LoggerManager::getInstance()
{
    static LoggerManager instance;
    return instance;
}

LoggerManager::LoggerManager()
    : myIsInitialized(false)
{}

LoggerManager::~LoggerManager()
{
    // Flush all loggers before shutdown
    spdlog::shutdown();
}

bool LoggerManager::initialize(const std::string& logDirPath)
{
    if (myIsInitialized) {
        return true;  // Already initialized
    }

    try {
        // Create log directory if it doesn't exist
        if (!createLogDirectory(logDirPath)) {
            std::cerr << "Failed to create log directory: " << logDirPath << std::endl;
            return false;
        }

        // Generate session ID based on current timestamp
        mySessionId = createSessionId();

        // Create log file path
        myLogFilePath = logDirPath + "/occt_imgui_" + mySessionId + ".log";

        // Create console sink
        myConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto console_sink =
            std::static_pointer_cast<spdlog::sinks::stdout_color_sink_mt>(myConsoleSink);

        // Set console sink level based on build configuration
#ifdef OCCTIMGUI_DEBUG
        console_sink->set_level(spdlog::level::debug);
#elif defined(OCCTIMGUI_RELWITHDEBINFO)
        console_sink->set_level(spdlog::level::debug);
#else
        console_sink->set_level(spdlog::level::info);
#endif
        console_sink->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");

        // Create file sink
        myFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(myLogFilePath,
                                                                            10 * 1024 * 1024,
                                                                            3,
                                                                            true);
        auto file_sink = std::static_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(myFileSink);
        file_sink->set_level(spdlog::level::debug);  // File records more detailed logs
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");

        // Create and configure root logger
        auto main_logger =
            std::make_shared<spdlog::logger>("main",
                                             spdlog::sinks_init_list {console_sink, file_sink});
        main_logger->set_level(spdlog::level::trace);
        main_logger->flush_on(spdlog::level::info);

        // Set as default logger
        spdlog::set_default_logger(main_logger);

        // Create root logger - this is the only logger we create by default
        myRootLogger = std::make_shared<Logger>("root");
        myRootLogger->setContextId(mySessionId);

        // Log session start
        spdlog::info("=====================================================");
        spdlog::info("OCCT ImGui Application Started - Session ID: {}", mySessionId);
        spdlog::info("=====================================================");

        myRootLogger->info("Logging system initialized");
        myIsInitialized = true;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to initialize logging system: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Unknown error during logging system initialization" << std::endl;
        return false;
    }
}

std::shared_ptr<Logger> LoggerManager::getLogger(const std::string& name)
{
    // Check if logger already exists
    for (const auto& pair : myLoggers) {
        if (pair.first == name) {
            return pair.second;
        }
    }

    // Create new logger if it doesn't exist
    return createLogger(name);
}

std::shared_ptr<Logger> LoggerManager::getRootLogger()
{
    return myRootLogger;
}

const std::string& LoggerManager::getSessionId() const
{
    return mySessionId;
}

const std::string& LoggerManager::getLogFilePath() const
{
    return myLogFilePath;
}

std::string LoggerManager::createSessionId()
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    char timeStr[100];
    std::strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S", std::localtime(&time_t_now));
    return std::string(timeStr);
}

std::shared_ptr<Logger> LoggerManager::createLogger(const std::string& name)
{
    auto logger = std::make_shared<Logger>(name);
    logger->setContextId(mySessionId);
    myLoggers.push_back(std::make_pair(name, logger));
    return logger;
}

bool LoggerManager::createLogDirectory(const std::string& logDirPath)
{
    try {
        std::filesystem::path dirPath(logDirPath);
        if (!std::filesystem::exists(dirPath)) {
            return std::filesystem::create_directory(dirPath);
        }
        return true;
    }
    catch (...) {
        return false;
    }
}

}  // namespace Utils