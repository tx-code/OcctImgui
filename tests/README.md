# 测试说明

本目录包含使用Boost.Test框架的单元测试。

## 运行测试

### 使用CMake和CTest

1. 首先构建项目：

```bash
mkdir -p build && cd build
cmake ..
make
```

2. 运行所有测试：

```bash
ctest
```

3. 运行特定测试：

```bash
ctest -R model_test        # 运行名称包含"model_test"的测试
ctest -R model_manager_test # 运行名称包含"model_manager_test"的测试
```

4. 查看详细输出：

```bash
ctest -V
```

### 直接运行测试可执行文件

你也可以直接运行测试可执行文件，这样可以获得更详细的输出：

```bash
./bin/Debug/model_test --log_level=all
./bin/Debug/model_manager_test --log_level=all
```

## Boost.Test命令行参数

Boost.Test支持多种命令行参数，以下是一些常用的：

- `--log_level=<level>`: 设置日志级别 (all, test_suite, message, warning, error, cpp_exception, system_error, fatal_error, nothing)
- `--run_test=<test_name>`: 只运行指定的测试用例
- `--report_level=<level>`: 设置报告级别 (no, confirm, short, detailed)
- `--show_progress`: 显示测试进度
- `--list_content`: 列出所有测试用例

例如：

```bash
./bin/Debug/model_test --log_level=all --run_test=simple_test
```

## 编写新测试

1. 在`tests`目录中创建新的测试文件，例如`my_feature_test.cpp`
2. 在文件顶部添加：

```cpp
#define BOOST_TEST_MODULE My Feature Tests
#include <boost/test/unit_test.hpp>
```

3. 编写测试用例：

```cpp
BOOST_AUTO_TEST_CASE(my_test_case)
{
    // 测试代码
    BOOST_CHECK(true);
}
```

4. 在`CMakeLists.txt`中添加测试：

```cmake
add_boost_test(my_feature_test tests/my_feature_test.cpp)
```

5. 重新构建项目并运行测试 