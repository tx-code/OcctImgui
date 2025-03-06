#define BOOST_TEST_MODULE MessageBus Tests
#include <boost/test/unit_test.hpp>

#include "mvvm/MessageBus.h"
#include <string>
#include <vector>

using namespace MVVM;

// Simple test case for basic MessageBus functionality
BOOST_AUTO_TEST_CASE(message_bus_basic_test)
{
    // Arrange
    MessageBus bus;
    int callCount = 0;
    std::string receivedData;

    // Act - subscribe to a message type
    bus.subscribe(MessageBus::MessageType::ModelChanged, [&](const MessageBus::Message& message) {
        callCount++;
        if (message.data.type() == typeid(std::string)) {
            receivedData = std::any_cast<std::string>(message.data);
        }
    });

    // Create and publish a message
    MessageBus::Message message;
    message.type = MessageBus::MessageType::ModelChanged;
    message.data = std::string("Model updated");

    bus.publish(message);

    // Assert
    BOOST_CHECK_EQUAL(callCount, 1);
    BOOST_CHECK_EQUAL(receivedData, "Model updated");
}

// Test multiple subscribers
BOOST_AUTO_TEST_CASE(message_bus_multiple_subscribers_test)
{
    // Arrange
    MessageBus bus;
    int subscriber1Count = 0;
    int subscriber2Count = 0;

    // Act - subscribe multiple handlers to the same message type
    bus.subscribe(MessageBus::MessageType::CommandExecuted, [&](const MessageBus::Message&) {
        subscriber1Count++;
    });

    bus.subscribe(MessageBus::MessageType::CommandExecuted, [&](const MessageBus::Message&) {
        subscriber2Count++;
    });

    // Create and publish a message
    MessageBus::Message message;
    message.type = MessageBus::MessageType::CommandExecuted;

    bus.publish(message);

    // Assert
    BOOST_CHECK_EQUAL(subscriber1Count, 1);
    BOOST_CHECK_EQUAL(subscriber2Count, 1);
}

// Test multiple message types
BOOST_AUTO_TEST_CASE(message_bus_multiple_message_types_test)
{
    // Arrange
    MessageBus bus;
    int modelChangedCount = 0;
    int viewChangedCount = 0;

    // Act - subscribe to different message types
    bus.subscribe(MessageBus::MessageType::ModelChanged, [&](const MessageBus::Message&) {
        modelChangedCount++;
    });

    bus.subscribe(MessageBus::MessageType::ViewChanged, [&](const MessageBus::Message&) {
        viewChangedCount++;
    });

    // Create and publish messages of different types
    MessageBus::Message modelMessage;
    modelMessage.type = MessageBus::MessageType::ModelChanged;

    MessageBus::Message viewMessage;
    viewMessage.type = MessageBus::MessageType::ViewChanged;

    bus.publish(modelMessage);
    bus.publish(viewMessage);
    bus.publish(modelMessage);

    // Assert
    BOOST_CHECK_EQUAL(modelChangedCount, 2);
    BOOST_CHECK_EQUAL(viewChangedCount, 1);
}

// Test complex data with SelectionInfo
BOOST_AUTO_TEST_CASE(message_bus_selection_info_test)
{
    // Arrange
    MessageBus bus;
    bool selectionReceived = false;
    SelectionInfo receivedInfo;

    // Act - subscribe to selection changed messages
    bus.subscribe(MessageBus::MessageType::SelectionChanged,
                  [&](const MessageBus::Message& message) {
                      if (message.data.type() == typeid(SelectionInfo)) {
                          selectionReceived = true;
                          receivedInfo = std::any_cast<SelectionInfo>(message.data);
                      }
                  });

    // Create selection info
    SelectionInfo selectionInfo;
    selectionInfo.selectionMode = 3;
    selectionInfo.selectionType = SelectionInfo::SelectionType::Add;

    // Add a sub-feature
    std::string objectId = "TestObject";
    SelectionInfo::SubFeatureIdentifier subFeature(SelectionInfo::SubFeatureType::Face, 42);
    selectionInfo.subFeatures[objectId].push_back(subFeature);

    // Create and publish message
    MessageBus::Message message;
    message.type = MessageBus::MessageType::SelectionChanged;
    message.data = selectionInfo;

    bus.publish(message);

    // Assert
    BOOST_CHECK(selectionReceived);
    BOOST_CHECK_EQUAL(receivedInfo.selectionMode, 3);
    BOOST_CHECK(receivedInfo.selectionType == SelectionInfo::SelectionType::Add);
    BOOST_CHECK(receivedInfo.subFeatures.count(objectId) > 0);
    BOOST_CHECK_EQUAL(receivedInfo.subFeatures[objectId].size(), 1);
    BOOST_CHECK(receivedInfo.subFeatures[objectId][0].type == SelectionInfo::SubFeatureType::Face);
    BOOST_CHECK_EQUAL(receivedInfo.subFeatures[objectId][0].index, 42);
}

// Test SubFeatureIdentifier with additional data
BOOST_AUTO_TEST_CASE(sub_feature_identifier_additional_data_test)
{
    // Arrange
    using SubFeatureType = SelectionInfo::SubFeatureType;

    // Act - create a sub-feature with additional data
    double paramU = 0.5;
    double paramV = 0.75;
    std::pair<double, double> uvParams(paramU, paramV);

    SelectionInfo::SubFeatureIdentifier subFeature(SubFeatureType::Face, 1, uvParams);

    // Assert
    BOOST_CHECK(subFeature.type == SubFeatureType::Face);
    BOOST_CHECK_EQUAL(subFeature.index, 1);
    BOOST_CHECK(subFeature.additionalData.has_value());

    // Extract and verify the additional data
    auto extractedParams = std::any_cast<std::pair<double, double>>(subFeature.additionalData);
    BOOST_CHECK_EQUAL(extractedParams.first, paramU);
    BOOST_CHECK_EQUAL(extractedParams.second, paramV);
}

// Test integration between MessageBus and different message types
BOOST_AUTO_TEST_CASE(message_bus_integration_test)
{
    // Arrange
    MessageBus bus;
    std::vector<std::string> receivedEvents;

    // Act - subscribe to all message types
    bus.subscribe(MessageBus::MessageType::ModelChanged, [&](const MessageBus::Message&) {
        receivedEvents.push_back("ModelChanged");
    });

    bus.subscribe(MessageBus::MessageType::ViewChanged, [&](const MessageBus::Message&) {
        receivedEvents.push_back("ViewChanged");
    });

    bus.subscribe(MessageBus::MessageType::SelectionChanged, [&](const MessageBus::Message&) {
        receivedEvents.push_back("SelectionChanged");
    });

    bus.subscribe(MessageBus::MessageType::CommandExecuted, [&](const MessageBus::Message&) {
        receivedEvents.push_back("CommandExecuted");
    });

    // Publish messages in a specific sequence
    MessageBus::Message message1;
    message1.type = MessageBus::MessageType::ModelChanged;
    bus.publish(message1);

    MessageBus::Message message2;
    message2.type = MessageBus::MessageType::SelectionChanged;
    bus.publish(message2);

    MessageBus::Message message3;
    message3.type = MessageBus::MessageType::CommandExecuted;
    bus.publish(message3);

    // Assert - check the sequence of received events
    BOOST_CHECK_EQUAL(receivedEvents.size(), 3);
    BOOST_CHECK_EQUAL(receivedEvents[0], "ModelChanged");
    BOOST_CHECK_EQUAL(receivedEvents[1], "SelectionChanged");
    BOOST_CHECK_EQUAL(receivedEvents[2], "CommandExecuted");
}

// 测试各种可调用对象
BOOST_AUTO_TEST_CASE(message_bus_callable_objects_test)
{
    // Arrange
    MessageBus bus;
    int lambdaCount = 0;
    int functionObjectCount = 0;
    int functionPointerCount = 0;
    int memberFunctionCount = 0;

    // 1. 使用 lambda 表达式
    bus.subscribe(MessageBus::MessageType::ModelChanged, [&](const MessageBus::Message&) {
        lambdaCount++;
    });

    // 2. 使用函数对象
    struct MessageHandler
    {
        int& counter;

        MessageHandler(int& c)
            : counter(c)
        {}

        void operator()(const MessageBus::Message&)
        {
            counter++;
        }
    };

    bus.subscribe(MessageBus::MessageType::ModelChanged, MessageHandler(functionObjectCount));

    // 3. 使用函数指针
    auto functionPointer = [&functionPointerCount](const MessageBus::Message&) -> void {
        functionPointerCount++;
    };

    bus.subscribe(MessageBus::MessageType::ModelChanged, functionPointer);

    // 4. 使用成员函数（通过 lambda 包装）
    class Observer
    {
    public:
        void handleMessage(const MessageBus::Message&)
        {
            memberFunctionCount++;
        }

        int& memberFunctionCount;

        Observer(int& count)
            : memberFunctionCount(count)
        {}
    };

    Observer observer(memberFunctionCount);
    bus.subscribe(MessageBus::MessageType::ModelChanged,
                  [&observer](const MessageBus::Message& msg) {
                      observer.handleMessage(msg);
                  });

    // Act - 发布消息
    MessageBus::Message message;
    message.type = MessageBus::MessageType::ModelChanged;
    bus.publish(message);

    // Assert - 所有处理器都应该被调用
    BOOST_CHECK_EQUAL(lambdaCount, 1);
    BOOST_CHECK_EQUAL(functionObjectCount, 1);
    BOOST_CHECK_EQUAL(functionPointerCount, 1);
    BOOST_CHECK_EQUAL(memberFunctionCount, 1);
}

// 测试可调用对象的状态捕获
BOOST_AUTO_TEST_CASE(message_bus_callable_state_capture_test)
{
    // Arrange
    MessageBus bus;
    std::vector<int> values;

    // 使用捕获外部状态的 lambda
    for (int i = 0; i < 3; i++) {
        // 每个 lambda 捕获不同的 i 值
        bus.subscribe(MessageBus::MessageType::CommandExecuted,
                      [i, &values](const MessageBus::Message&) {
                          values.push_back(i * 10);
                      });
    }

    // Act - 发布消息
    MessageBus::Message message;
    message.type = MessageBus::MessageType::CommandExecuted;
    bus.publish(message);

    // Assert - 检查每个 lambda 是否正确捕获并使用了其状态
    BOOST_CHECK_EQUAL(values.size(), 3);
    BOOST_CHECK_EQUAL(values[0], 0);
    BOOST_CHECK_EQUAL(values[1], 10);
    BOOST_CHECK_EQUAL(values[2], 20);
}

// 测试带有复杂参数的可调用对象
BOOST_AUTO_TEST_CASE(message_bus_complex_callable_test)
{
    // Arrange
    MessageBus bus;
    std::map<std::string, int> dataMap;

    // 使用复杂的 lambda，处理不同类型的数据
    bus.subscribe(MessageBus::MessageType::ModelChanged,
                  [&dataMap](const MessageBus::Message& message) {
                      if (message.data.type() == typeid(int)) {
                          dataMap["int"] = std::any_cast<int>(message.data);
                      }
                      else if (message.data.type() == typeid(std::string)) {
                          dataMap["string"] =
                              static_cast<int>(std::any_cast<std::string>(message.data).length());
                      }
                      else if (message.data.type() == typeid(double)) {
                          dataMap["double"] = static_cast<int>(std::any_cast<double>(message.data));
                      }
                  });

    // Act - 发布不同类型的消息
    MessageBus::Message intMessage;
    intMessage.type = MessageBus::MessageType::ModelChanged;
    intMessage.data = 42;
    bus.publish(intMessage);

    MessageBus::Message stringMessage;
    stringMessage.type = MessageBus::MessageType::ModelChanged;
    stringMessage.data = std::string("Hello");
    bus.publish(stringMessage);

    MessageBus::Message doubleMessage;
    doubleMessage.type = MessageBus::MessageType::ModelChanged;
    doubleMessage.data = 3.14;
    bus.publish(doubleMessage);

    // Assert - 检查是否正确处理了所有类型
    BOOST_CHECK_EQUAL(dataMap["int"], 42);
    BOOST_CHECK_EQUAL(dataMap["string"], 5);  // "Hello" 的长度
    BOOST_CHECK_EQUAL(dataMap["double"], 3);  // 3.14 转为 int
}