#pragma once

#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace Utils {

/**
 * @brief 分层级日志记录器
 * 
 * 提供分层级的日志记录功能，可以跟踪调用链和上下文
 */
class Logger : public std::enable_shared_from_this<Logger> {
public:
    /**
     * @brief 获取指定模块的日志记录器
     * 
     * @param module 模块名称
     * @return 日志记录器实例
     */
    static std::shared_ptr<Logger> getLogger(const std::string& module);

    /**
     * @brief 构造函数
     * 
     * @param module 模块名称
     */
    Logger(const std::string& module) : myModule(module) {}

    /**
     * @brief 创建子日志记录器
     * 
     * @param subModule 子模块名称
     * @return 子日志记录器
     */
    std::shared_ptr<Logger> createChild(const std::string& subModule) {
        std::string fullPath = myModule + "." + subModule;
        return std::make_shared<Logger>(fullPath);
    }

    /**
     * @brief 设置上下文ID
     * 
     * @param contextId 上下文ID
     */
    void setContextId(const std::string& contextId) {
        myContextId = contextId;
    }

    /**
     * @brief 获取完整的日志前缀
     * 
     * @return 日志前缀
     */
    std::string getPrefix() const {
        if (myContextId.empty()) {
            return "[" + myModule + "]";
        } else {
            return "[" + myModule + ":" + myContextId + "]";
        }
    }

    // 各级别日志记录函数
    template<typename... Args>
    void trace(const std::string& fmt, const Args&... args) {
        auto logger = spdlog::get(myModule);
        if (logger) {
            logger->trace(getPrefix() + " " + fmt, args...);
        } else {
            spdlog::trace(getPrefix() + " " + fmt, args...);
        }
    }

    template<typename... Args>
    void debug(const std::string& fmt, const Args&... args) {
        auto logger = spdlog::get(myModule);
        if (logger) {
            logger->debug(getPrefix() + " " + fmt, args...);
        } else {
            spdlog::debug(getPrefix() + " " + fmt, args...);
        }
    }

    template<typename... Args>
    void info(const std::string& fmt, const Args&... args) {
        auto logger = spdlog::get(myModule);
        if (logger) {
            logger->info(getPrefix() + " " + fmt, args...);
        } else {
            spdlog::info(getPrefix() + " " + fmt, args...);
        }
    }

    template<typename... Args>
    void warn(const std::string& fmt, const Args&... args) {
        auto logger = spdlog::get(myModule);
        if (logger) {
            logger->warn(getPrefix() + " " + fmt, args...);
        } else {
            spdlog::warn(getPrefix() + " " + fmt, args...);
        }
    }

    template<typename... Args>
    void error(const std::string& fmt, const Args&... args) {
        auto logger = spdlog::get(myModule);
        if (logger) {
            logger->error(getPrefix() + " " + fmt, args...);
        } else {
            spdlog::error(getPrefix() + " " + fmt, args...);
        }
    }

    template<typename... Args>
    void critical(const std::string& fmt, const Args&... args) {
        auto logger = spdlog::get(myModule);
        if (logger) {
            logger->critical(getPrefix() + " " + fmt, args...);
        } else {
            spdlog::critical(getPrefix() + " " + fmt, args...);
        }
    }

    /**
     * @brief 记录函数进入
     * 
     * @param functionName 函数名称
     * @return 函数作用域对象，在析构时自动记录函数退出
     */
    class ScopedLogger {
    public:
        ScopedLogger(std::shared_ptr<Logger> logger, const std::string& functionName)
            : myLogger(logger), myFunctionName(functionName) {
            myLogger->debug("Enter: {}", myFunctionName);
        }

        ~ScopedLogger() {
            myLogger->debug("Exit: {}", myFunctionName);
        }

    private:
        std::shared_ptr<Logger> myLogger;
        std::string myFunctionName;
    };

    ScopedLogger functionScope(const std::string& functionName) {
        return ScopedLogger(shared_from_this(), functionName);
    }

private:
    std::string myModule;
    std::string myContextId;
};

// 便捷宏，用于创建函数作用域日志
#define LOG_FUNCTION_SCOPE(logger, function) auto scopedLogger = logger->functionScope(function)

} // namespace Utils 