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

#include "GlfwOcctView.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

void initLogging()
{
    try {
        // 创建控制台和文件日志接收器
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug); // 控制台显示debug以上级别
        
        // 创建一个最大10MB、3个旋转文件的文件日志接收器
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/occt_app.log", 10 * 1024 * 1024, 3);
        file_sink->set_level(spdlog::level::trace); // 文件记录所有日志
        
        // 创建带多个接收器的日志器
        auto logger = std::make_shared<spdlog::logger>(
            "multi_sink", spdlog::sinks_init_list{console_sink, file_sink});
        
        // 设置RelWithDebInfo模式下的默认日志级别
        logger->set_level(spdlog::level::debug);
        
        // 设置为默认日志器
        spdlog::set_default_logger(logger);
        
        // 设置日志模式
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
        
        spdlog::info("Logging system initialized");
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

int main(int argc, char** argv)
{
    // 初始化日志
    initLogging();

    GlfwOcctView anApp;

    try {
        anApp.run();
    }
    catch (const std::runtime_error& theError) {
        std::cerr << theError.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
