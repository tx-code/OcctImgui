#define BOOST_TEST_MODULE Core Property Tests
#include <boost/test/unit_test.hpp>

#include "core/Property.h"
#include <memory>
#include <string>
#include <vector>

#include <Eigen/Dense>

using namespace Core;

// 测试基本的 Property 功能
BOOST_AUTO_TEST_CASE(property_basic_test)
{
    // 准备
    Property<int> property(42);

    // 验证初始值
    BOOST_CHECK_EQUAL(property.get(), 42);

    // 测试 set 方法
    bool changed = property.set(100);
    BOOST_CHECK(changed);
    BOOST_CHECK_EQUAL(property.get(), 100);

    // 测试设置相同的值
    changed = property.set(100);
    BOOST_CHECK(!changed);  // 值没有改变，应该返回 false
}

// 测试 Property 的值变化信号
BOOST_AUTO_TEST_CASE(property_value_changed_signal_test)
{
    // 准备
    Property<int> property(0);
    int callCount = 0;
    int lastOldValue = 0;
    int lastNewValue = 0;

    // 连接信号
    auto connection = property.valueChanged.connect([&](const int& oldValue, const int& newValue) {
        callCount++;
        lastOldValue = oldValue;
        lastNewValue = newValue;
    });

    // 改变属性值
    property.set(42);

    // 验证信号被触发
    BOOST_CHECK_EQUAL(callCount, 1);
    BOOST_CHECK_EQUAL(lastOldValue, 0);
    BOOST_CHECK_EQUAL(lastNewValue, 42);

    // 断开连接
    connection.disconnect();

    // 再次改变属性值
    property.set(100);

    // 验证信号不再被触发
    BOOST_CHECK_EQUAL(callCount, 1);  // 仍然是 1，没有增加
}

// 测试 Property 的赋值运算符
BOOST_AUTO_TEST_CASE(property_assignment_operator_test)
{
    // 准备
    Property<int> property(0);
    int callCount = 0;

    // 连接信号
    auto connection = property.valueChanged.connect([&](const int&, const int&) {
        callCount++;
    });

    // 使用赋值运算符
    property = 42;

    // 验证
    BOOST_CHECK_EQUAL(property.get(), 42);
    BOOST_CHECK_EQUAL(callCount, 1);
}

// 测试 Property 的类型转换运算符
BOOST_AUTO_TEST_CASE(property_conversion_operator_test)
{
    // 准备
    Property<int> property(42);

    // 使用类型转换运算符
    int value = property;

    // 验证
    BOOST_CHECK_EQUAL(value, 42);
}

// 测试 Property 的绑定功能
BOOST_AUTO_TEST_CASE(property_binding_test)
{
    // 准备
    Property<std::string> source("Hello");
    Property<std::string> target;

    // 绑定 target 到 source
    auto connection = target.bindTo(source);

    // 验证初始绑定
    BOOST_CHECK_EQUAL(target.get(), "Hello");

    // 改变 source
    source.set("World");

    // 验证 target 也改变了
    BOOST_CHECK_EQUAL(target.get(), "World");

    // 断开绑定
    connection.disconnect();

    // 再次改变 source
    source.set("Test");

    // 验证 target 不再改变
    BOOST_CHECK_EQUAL(target.get(), "World");
}

// 测试 Property 的计算绑定功能
BOOST_AUTO_TEST_CASE(property_computed_binding_test)
{
    // 准备
    Property<int> width(5);
    Property<int> height(10);
    Property<int> area;

    // 创建计算绑定
    auto connections = area.bindComputed(
        [](const int& w, const int& h) {
            return w * h;
        },
        width,
        height);

    // 验证初始计算
    BOOST_CHECK_EQUAL(area.get(), 50);  // 5 * 10 = 50

    // 改变一个依赖项
    width.set(7);

    // 验证重新计算
    BOOST_CHECK_EQUAL(area.get(), 70);  // 7 * 10 = 70

    // 改变另一个依赖项
    height.set(8);

    // 验证再次重新计算
    BOOST_CHECK_EQUAL(area.get(), 56);  // 7 * 8 = 56
}

// 测试 Property 的序列化功能
BOOST_AUTO_TEST_CASE(property_serialization_test)
{
    Property<int> property(42);

    // 序列化
    auto json = property.toJson();
    BOOST_CHECK(json.is_number_integer());
    BOOST_CHECK_EQUAL(json.get<int>(), 42);

    // 反序列化
    Property<int> deserializedProperty;
    BOOST_CHECK(deserializedProperty.fromJson(json));
    BOOST_CHECK_EQUAL(deserializedProperty.get(), 42);
}
