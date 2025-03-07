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
#include <iostream>

int main(int, char**)
{
    try {
        // 初始化日志系统
        if (!Utils::Logger::initialize()) {
            std::cerr << "Failed to initialize logging system" << std::endl;
            return EXIT_FAILURE;
        }

        // 获取根日志记录器
        auto rootLogger = Utils::Logger::getLogger("root");
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
