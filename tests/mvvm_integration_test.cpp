#define BOOST_TEST_MODULE MVVM Integration Tests
#include <boost/test/unit_test.hpp>

#include "mvvm/MessageBus.h"
#include "mvvm/Signal.h"
#include <memory>
#include <string>
#include <vector>


using namespace MVVM;

// A simple model class that uses both Signal and MessageBus
class TestModel
{
public:
    TestModel(MessageBus& messageBus)
        : myMessageBus(messageBus)
    {}

    // Signal for direct connections
    Signal<int> valueChanged;

    void setValue(int newValue)
    {
        if (myValue != newValue) {
            myValue = newValue;

            // Emit direct signal
            valueChanged.emit(myValue);

            // Publish to message bus
            MessageBus::Message message;
            message.type = MessageBus::MessageType::ModelChanged;
            message.data = std::string("Value changed to " + std::to_string(myValue));
            myMessageBus.publish(message);
        }
    }

    int getValue() const
    {
        return myValue;
    }

private:
    int myValue = 0;
    MessageBus& myMessageBus;
};

// A simple view class that listens to both Signal and MessageBus
class TestView
{
public:
    TestView(MessageBus& messageBus)
        : myMessageBus(messageBus)
    {
        // Subscribe to message bus
        myToken = myMessageBus.subscribe(MessageBus::MessageType::ModelChanged,
                                         [this](const MessageBus::Message& message) {
                                             if (message.data.type() == typeid(std::string)) {
                                                 lastMessageReceived =
                                                     std::any_cast<std::string>(message.data);
                                                 messageCount++;
                                             }
                                         });
    }

    ~TestView()
    {
        myMessageBus.unsubscribe(myToken);
    }

    // Connect to model's signal
    void connectToModel(TestModel& model)
    {
        // Use ConnectionTracker to manage connection lifetime
        myConnections.track(model.valueChanged.connect([this](int newValue) {
            lastValueReceived = newValue;
            signalCount++;
        }));
    }

    int signalCount = 0;
    int messageCount = 0;
    int lastValueReceived = 0;
    std::string lastMessageReceived;

private:
    MessageBus& myMessageBus;
    ConnectionTracker myConnections;
    int myToken;
};

// Test the integration between Signal and MessageBus
BOOST_AUTO_TEST_CASE(signal_message_bus_integration_test)
{
    // Arrange
    auto& messageBus = MessageBus::getInstance();
    TestModel model(messageBus);
    TestView view(messageBus);

    // Connect view to model
    view.connectToModel(model);

    // Act
    model.setValue(42);

    // Assert - both signal and message bus notifications should be received
    BOOST_CHECK_EQUAL(view.signalCount, 1);
    BOOST_CHECK_EQUAL(view.messageCount, 1);
    BOOST_CHECK_EQUAL(view.lastValueReceived, 42);
    BOOST_CHECK_EQUAL(view.lastMessageReceived, "Value changed to 42");

    // Act again
    model.setValue(100);

    // Assert
    BOOST_CHECK_EQUAL(view.signalCount, 2);
    BOOST_CHECK_EQUAL(view.messageCount, 2);
    BOOST_CHECK_EQUAL(view.lastValueReceived, 100);
    BOOST_CHECK_EQUAL(view.lastMessageReceived, "Value changed to 100");
}

// Test multiple views with one model
BOOST_AUTO_TEST_CASE(multiple_views_test)
{
    // Arrange
    auto& messageBus = MessageBus::getInstance();
    TestModel model(messageBus);
    TestView view1(messageBus);
    TestView view2(messageBus);

    // Connect views to model
    view1.connectToModel(model);
    view2.connectToModel(model);

    // Act
    model.setValue(42);

    // Assert - both views should receive notifications
    BOOST_CHECK_EQUAL(view1.signalCount, 1);
    BOOST_CHECK_EQUAL(view1.messageCount, 1);
    BOOST_CHECK_EQUAL(view1.lastValueReceived, 42);

    BOOST_CHECK_EQUAL(view2.signalCount, 1);
    BOOST_CHECK_EQUAL(view2.messageCount, 1);
    BOOST_CHECK_EQUAL(view2.lastValueReceived, 42);
}

// Test MVVM pattern with Signal and MessageBus
class ViewModel
{
public:
    ViewModel(std::shared_ptr<TestModel> model, MessageBus& messageBus)
        : myModel(model)
        , myMessageBus(messageBus)
    {

        // Connect to model's signal
        myConnections.track(myModel->valueChanged.connect([this](int newValue) {
            // Process the value and emit our own signal
            std::string formattedValue = "Value: " + std::to_string(newValue);
            displayTextChanged.emit(formattedValue);

            // Also publish to message bus
            MessageBus::Message message;
            message.type = MessageBus::MessageType::ViewChanged;
            message.data = formattedValue;
            myMessageBus.publish(message);
        }));
    }

    // Signal for view binding
    Signal<std::string> displayTextChanged;

private:
    std::shared_ptr<TestModel> myModel;
    MessageBus& myMessageBus;
    ConnectionTracker myConnections;
};

BOOST_AUTO_TEST_CASE(mvvm_pattern_test)
{
    // Arrange
    auto& messageBus = MessageBus::getInstance();
    auto model = std::make_shared<TestModel>(messageBus);
    auto viewModel = std::make_shared<ViewModel>(model, messageBus);

    std::string lastDisplayText;
    int displayTextChangedCount = 0;

    // Connect to ViewModel's signal
    ScopedConnection connection(viewModel->displayTextChanged.connect([&](const std::string& text) {
        lastDisplayText = text;
        displayTextChangedCount++;
    }));

    // Track ViewChanged messages
    std::string lastViewChangedMessage;
    int viewChangedCount = 0;
    messageBus.subscribe(MessageBus::MessageType::ViewChanged,
                         [&](const MessageBus::Message& message) {
                             if (message.data.type() == typeid(std::string)) {
                                 lastViewChangedMessage = std::any_cast<std::string>(message.data);
                                 viewChangedCount++;
                             }
                         });

    // Act
    model->setValue(42);

    // Assert
    BOOST_CHECK_EQUAL(displayTextChangedCount, 1);
    BOOST_CHECK_EQUAL(lastDisplayText, "Value: 42");
    BOOST_CHECK_EQUAL(viewChangedCount, 1);
    BOOST_CHECK_EQUAL(lastViewChangedMessage, "Value: 42");
}