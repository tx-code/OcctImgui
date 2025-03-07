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
    auto& bus = MessageBus::getInstance();
    int callCount = 0;
    std::string receivedData;

    // Act - subscribe to a message type
    auto token = bus.subscribe(MessageBus::MessageType::ModelChanged,
                               [&](const MessageBus::Message& message) {
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

    // Clean up
    bus.unsubscribe(token);
}

// Test multiple subscribers
BOOST_AUTO_TEST_CASE(message_bus_multiple_subscribers_test)
{
    // Arrange
    auto& bus = MessageBus::getInstance();
    int subscriber1Count = 0;
    int subscriber2Count = 0;

    // Act - subscribe multiple handlers to the same message type
    auto token1 =
        bus.subscribe(MessageBus::MessageType::CommandExecuted, [&](const MessageBus::Message&) {
            subscriber1Count++;
        });

    auto token2 =
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

    // Clean up
    bus.unsubscribe(token1);
    bus.unsubscribe(token2);
}

// Test multiple message types
BOOST_AUTO_TEST_CASE(message_bus_multiple_message_types_test)
{
    // Arrange
    auto& bus = MessageBus::getInstance();
    int modelChangedCount = 0;
    int viewChangedCount = 0;

    // Act - subscribe to different message types
    auto token1 =
        bus.subscribe(MessageBus::MessageType::ModelChanged, [&](const MessageBus::Message&) {
            modelChangedCount++;
        });

    auto token2 =
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

    // Clean up
    bus.unsubscribe(token1);
    bus.unsubscribe(token2);
}

// Test unsubscribe functionality
BOOST_AUTO_TEST_CASE(message_bus_unsubscribe_test)
{
    // Arrange
    auto& bus = MessageBus::getInstance();
    int callCount = 0;

    // Act - subscribe to a message type
    auto token =
        bus.subscribe(MessageBus::MessageType::ModelChanged, [&](const MessageBus::Message&) {
            callCount++;
        });

    // Create a message
    MessageBus::Message message;
    message.type = MessageBus::MessageType::ModelChanged;

    // Publish once, then unsubscribe, then publish again
    bus.publish(message);
    bool unsubscribeResult = bus.unsubscribe(token);
    bus.publish(message);

    // Assert
    BOOST_CHECK_EQUAL(callCount, 1);  // Should only be called once
    BOOST_CHECK(unsubscribeResult);   // Unsubscribe should return true

    // Try to unsubscribe again with the same token
    bool secondUnsubscribeResult = bus.unsubscribe(token);
    BOOST_CHECK(!secondUnsubscribeResult);  // Should return false as already unsubscribed
}

// Test complex data with SelectionInfo
BOOST_AUTO_TEST_CASE(message_bus_selection_info_test)
{
    // Arrange
    auto& bus = MessageBus::getInstance();
    bool selectionReceived = false;
    SelectionInfo receivedInfo;

    // Act - subscribe to selection changed messages
    auto token = bus.subscribe(MessageBus::MessageType::SelectionChanged,
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

    // Clean up
    bus.unsubscribe(token);
}

// Test integration between MessageBus and different message types
BOOST_AUTO_TEST_CASE(message_bus_integration_test)
{
    // Arrange
    auto& bus = MessageBus::getInstance();
    std::vector<std::string> receivedEvents;

    // Act - subscribe to all message types
    auto token1 =
        bus.subscribe(MessageBus::MessageType::ModelChanged, [&](const MessageBus::Message&) {
            receivedEvents.push_back("ModelChanged");
        });

    auto token2 =
        bus.subscribe(MessageBus::MessageType::ViewChanged, [&](const MessageBus::Message&) {
            receivedEvents.push_back("ViewChanged");
        });

    auto token3 =
        bus.subscribe(MessageBus::MessageType::SelectionChanged, [&](const MessageBus::Message&) {
            receivedEvents.push_back("SelectionChanged");
        });

    auto token4 =
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

    // Clean up
    bus.unsubscribe(token1);
    bus.unsubscribe(token2);
    bus.unsubscribe(token3);
    bus.unsubscribe(token4);
}

// Test clearAllSubscriptions
BOOST_AUTO_TEST_CASE(message_bus_clear_all_subscriptions_test)
{
    // Arrange
    auto& bus = MessageBus::getInstance();
    int callCount1 = 0;
    int callCount2 = 0;

    // Subscribe to different message types
    bus.subscribe(MessageBus::MessageType::ModelChanged, [&](const MessageBus::Message&) {
        callCount1++;
    });

    bus.subscribe(MessageBus::MessageType::ViewChanged, [&](const MessageBus::Message&) {
        callCount2++;
    });

    // Create messages
    MessageBus::Message message1;
    message1.type = MessageBus::MessageType::ModelChanged;

    MessageBus::Message message2;
    message2.type = MessageBus::MessageType::ViewChanged;

    // Publish once
    bus.publish(message1);
    bus.publish(message2);

    // Clear all subscriptions
    bus.clearAllSubscriptions();

    // Publish again
    bus.publish(message1);
    bus.publish(message2);

    // Assert - should only be called once each before clearing
    BOOST_CHECK_EQUAL(callCount1, 1);
    BOOST_CHECK_EQUAL(callCount2, 1);
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

// 测试各种可调用对象
BOOST_AUTO_TEST_CASE(message_bus_callable_objects_test)
{
    // Arrange
    auto& bus = MessageBus::getInstance();
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

    // Clean up
    bus.clearAllSubscriptions();
}

// 测试可调用对象的状态捕获
BOOST_AUTO_TEST_CASE(message_bus_callable_state_capture_test)
{
    // Arrange
    auto& bus = MessageBus::getInstance();
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

    bus.clearAllSubscriptions();
}

// 测试带有复杂参数的可调用对象
BOOST_AUTO_TEST_CASE(message_bus_complex_callable_test)
{
    // Arrange
    auto& bus = MessageBus::getInstance();
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

    bus.clearAllSubscriptions();
}

// Test Subscription class for automatic unsubscribe
BOOST_AUTO_TEST_CASE(message_bus_subscription_class_test)
{
    // Arrange
    auto& bus = MessageBus::getInstance();
    int callCount = 0;

    // Create a scope to test automatic unsubscribe
    {
        // Create a subscription that will go out of scope
        auto subscription = bus.subscribeWithManager(MessageBus::MessageType::ModelChanged,
                                                     [&](const MessageBus::Message&) {
                                                         callCount++;
                                                     });

        // Publish a message while subscription is active
        MessageBus::Message message;
        message.type = MessageBus::MessageType::ModelChanged;
        bus.publish(message);

        // Check that the handler was called
        BOOST_CHECK_EQUAL(callCount, 1);

        // Verify subscription is active
        BOOST_CHECK(subscription.hasActiveSubscriptions());
    }
    // Subscription is now out of scope and should be automatically unsubscribed

    // Publish another message
    MessageBus::Message message;
    message.type = MessageBus::MessageType::ModelChanged;
    bus.publish(message);

    // The handler should not be called again
    BOOST_CHECK_EQUAL(callCount, 1);
}

// Test manual unsubscribe with Subscription class
BOOST_AUTO_TEST_CASE(message_bus_subscription_manual_unsubscribe_test)
{
    // Arrange
    auto& bus = MessageBus::getInstance();
    int callCount = 0;

    // Create a subscription
    auto subscription = bus.subscribeWithManager(MessageBus::MessageType::ModelChanged,
                                                 [&](const MessageBus::Message&) {
                                                     callCount++;
                                                 });

    // Publish a message
    MessageBus::Message message;
    message.type = MessageBus::MessageType::ModelChanged;
    bus.publish(message);

    // Check that the handler was called
    BOOST_CHECK_EQUAL(callCount, 1);

    // Manually unsubscribe
    subscription.unsubscribe();

    // Verify subscription is no longer active
    BOOST_CHECK(!subscription.hasActiveSubscriptions());

    // Publish another message
    bus.publish(message);

    // The handler should not be called again
    BOOST_CHECK_EQUAL(callCount, 1);
}

// Test multiple message types with a single subscription
BOOST_AUTO_TEST_CASE(message_bus_multiple_types_subscription_test)
{
    // Arrange
    auto& bus = MessageBus::getInstance();
    std::vector<std::string> receivedEvents;

    // Subscribe to multiple message types with a single handler
    auto subscription =
        bus.subscribeMultiple({MessageBus::MessageType::ModelChanged,
                               MessageBus::MessageType::ViewChanged,
                               MessageBus::MessageType::SelectionChanged},
                              [&](const MessageBus::Message& message) {
                                  switch (message.type) {
                                      case MessageBus::MessageType::ModelChanged:
                                          receivedEvents.push_back("ModelChanged");
                                          break;
                                      case MessageBus::MessageType::ViewChanged:
                                          receivedEvents.push_back("ViewChanged");
                                          break;
                                      case MessageBus::MessageType::SelectionChanged:
                                          receivedEvents.push_back("SelectionChanged");
                                          break;
                                      default:
                                          break;
                                  }
                              });

    // Verify we have multiple tokens
    BOOST_CHECK_EQUAL(subscription.getTokens().size(), 3);

    // Publish messages of different types
    MessageBus::Message modelMessage;
    modelMessage.type = MessageBus::MessageType::ModelChanged;
    bus.publish(modelMessage);

    MessageBus::Message viewMessage;
    viewMessage.type = MessageBus::MessageType::ViewChanged;
    bus.publish(viewMessage);

    MessageBus::Message selectionMessage;
    selectionMessage.type = MessageBus::MessageType::SelectionChanged;
    bus.publish(selectionMessage);

    // Check that all messages were received
    BOOST_CHECK_EQUAL(receivedEvents.size(), 3);
    BOOST_CHECK_EQUAL(receivedEvents[0], "ModelChanged");
    BOOST_CHECK_EQUAL(receivedEvents[1], "ViewChanged");
    BOOST_CHECK_EQUAL(receivedEvents[2], "SelectionChanged");

    // Test unsubscribing a single token
    auto tokens = subscription.getTokens();
    BOOST_CHECK(subscription.unsubscribeToken(tokens[0]));

    // Now we should have 2 tokens left
    BOOST_CHECK_EQUAL(subscription.getTokens().size(), 2);

    // Clear events and publish again
    receivedEvents.clear();

    // Model message should not be received anymore
    bus.publish(modelMessage);
    bus.publish(viewMessage);

    BOOST_CHECK_EQUAL(receivedEvents.size(), 1);
    BOOST_CHECK_EQUAL(receivedEvents[0], "ViewChanged");

    // Unsubscribe all remaining tokens
    subscription.unsubscribe();

    // Verify no tokens left
    BOOST_CHECK_EQUAL(subscription.getTokens().size(), 0);
    BOOST_CHECK(!subscription.hasActiveSubscriptions());

    // No more messages should be received
    receivedEvents.clear();
    bus.publish(viewMessage);
    bus.publish(selectionMessage);

    BOOST_CHECK_EQUAL(receivedEvents.size(), 0);
}