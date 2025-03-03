#include "Logger.h"

namespace Utils {

// 使用Meyer's Singleton模式确保安全初始化
std::unordered_map<std::string, std::shared_ptr<Logger>>& getLoggerRegistry() {
    static std::unordered_map<std::string, std::shared_ptr<Logger>> registry;
    return registry;
}

// 获取或创建日志记录器
std::shared_ptr<Logger> Logger::getLogger(const std::string& module) {
    auto& registry = getLoggerRegistry();
    auto it = registry.find(module);
    if (it != registry.end()) {
        return it->second;
    }
    
    auto logger = std::make_shared<Logger>(module);
    registry[module] = logger;
    return logger;
}

} // namespace Utils 