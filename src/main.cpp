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
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
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
        
        // 获取时间戳作为日志文件名
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        char timeStr[100];
        std::strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S", std::localtime(&time_t_now));
        std::string logFileName = "logs/occt_imgui_" + std::string(timeStr) + ".log";
        
        // 创建控制台日志接收器 - 仅英文日志
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);
        console_sink->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
        
        // 创建文件日志接收器 - 可包含中文
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileName, true);
        file_sink->set_level(spdlog::level::info);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        
        // 创建和配置多接收器日志记录器
        auto logger = std::make_shared<spdlog::logger>("occt_imgui", 
            spdlog::sinks_init_list{console_sink, file_sink});
        logger->set_level(spdlog::level::info);
        logger->flush_on(spdlog::level::info);
        
        // 设置为默认日志记录器
        spdlog::set_default_logger(logger);
        
        // 控制台使用ASCII字符
        spdlog::info("=====================================================");
        spdlog::info("OCCT ImGui Application Started");
        spdlog::info("=====================================================");
        
        Application app;
        app.run();
    }
    catch (const std::runtime_error& theError) {
        spdlog::error("Runtime error: {}", theError.what());
        std::cerr << theError.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception& e) {
        spdlog::error("Unhandled exception: {}", e.what());
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        spdlog::error("Unknown error");
        std::cerr << "Unknown error" << std::endl;
        return EXIT_FAILURE;
    }

    spdlog::info("Application exited normally");
    return EXIT_SUCCESS;
}
