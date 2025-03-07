#include "Logger.h"
#include "LoggerManager.h"

namespace Utils
{

// 使用Meyer's Singleton模式确保安全初始化
std::unordered_map<std::string, std::shared_ptr<Logger>>& getLoggerRegistry()
{
    static std::unordered_map<std::string, std::shared_ptr<Logger>> registry;
    return registry;
}

// 获取或创建日志记录器
std::shared_ptr<Logger> Logger::getLogger(const std::string& module)
{
    return LoggerManager::getInstance().getLogger(module);
}

bool Logger::initialize(const std::string& logDirPath)
{
    return LoggerManager::getInstance().initialize(logDirPath);
}

Logger::Logger(const std::string& module)
    : myModule(module)
{}

std::shared_ptr<Logger> Logger::createChild(const std::string& subModule)
{
    std::string fullPath = myModule + "." + subModule;
    return std::make_shared<Logger>(fullPath);
}

void Logger::setContextId(const std::string& contextId)
{
    myContextId = contextId;
}

std::string Logger::getPrefix() const
{
    return "[" + myModule + "]";
}

Logger::ScopedLogger::ScopedLogger(std::shared_ptr<Logger> logger, const std::string& functionName)
    : myLogger(logger)
    , myFunctionName(functionName)
{
    myLogger->debug("Enter: {}", myFunctionName);
}

Logger::ScopedLogger::~ScopedLogger()
{
    myLogger->debug("Exit: {}", myFunctionName);
}

Logger::ScopedLogger Logger::functionScope(const std::string& functionName)
{
    return ScopedLogger(shared_from_this(), functionName);
}

}  // namespace Utils