// MIT License
//
// Copyright(c) 2023 Shing Liu
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "Application.h"
#include "utils/Logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <filesystem>
#include <chrono>
#include <iostream>

int main(int, char**)
{
    try {
        // 创建日志目录（如果不存在）
        std::filesystem::path logDir = "logs";
        if (!std::filesystem::exists(logDir)) {
            std::filesystem::create_directory(logDir);
        }
        
        // 获取时间戳作为日志文件名和会话ID
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        char timeStr[100];
        std::strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S", std::localtime(&time_t_now));
        std::string sessionId = std::string(timeStr);
        std::string logFileName = "logs/occt_imgui_" + sessionId + ".log";
        
        // 创建控制台日志接收器
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);
        console_sink->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
        
        // 创建文件日志接收器
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileName, true);
        file_sink->set_level(spdlog::level::debug);  // 文件记录更详细的日志
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
        
        // 创建和配置多接收器日志记录器
        auto main_logger = std::make_shared<spdlog::logger>("main", 
            spdlog::sinks_init_list{console_sink, file_sink});
        main_logger->set_level(spdlog::level::trace);
        main_logger->flush_on(spdlog::level::info);
        
        // 设置为默认日志记录器
        spdlog::set_default_logger(main_logger);
        
        // 创建各子系统的日志记录器
        auto create_logger = [&](const std::string& name) {
            auto logger = std::make_shared<spdlog::logger>(name, 
                spdlog::sinks_init_list{console_sink, file_sink});
            logger->set_level(spdlog::level::trace);
            logger->flush_on(spdlog::level::info);
            spdlog::register_logger(logger);
        };
        
        // 创建各子系统的日志记录器
        create_logger("app");
        create_logger("occt");
        create_logger("imgui");
        create_logger("model");
        create_logger("view");
        create_logger("viewmodel");
        
        // 记录会话开始信息
        spdlog::info("=====================================================");
        spdlog::info("OCCT ImGui Application Started - Session ID: {}", sessionId);
        spdlog::info("=====================================================");
        
        // 使用新的分层级日志系统
        auto rootLogger = Utils::Logger::getLogger("root");
        rootLogger->setContextId(sessionId);
        rootLogger->info("Application starting");
        
        // 使用函数作用域日志
        {
            LOG_FUNCTION_SCOPE(rootLogger, "main");
            
            // 启动应用程序
            Application app;
            app.run();
        }
    }
    catch (const std::runtime_error& theError) {
        auto rootLogger = Utils::Logger::getLogger("root");
        rootLogger->error("Runtime error: {}", theError.what());
        std::cerr << theError.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception& e) {
        auto rootLogger = Utils::Logger::getLogger("root");
        rootLogger->error("Unhandled exception: {}", e.what());
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        auto rootLogger = Utils::Logger::getLogger("root");
        rootLogger->error("Unknown error");
        std::cerr << "Unknown error" << std::endl;
        return EXIT_FAILURE;
    }

    auto rootLogger = Utils::Logger::getLogger("root");
    rootLogger->info("Application exited normally");
    return EXIT_SUCCESS;
}
