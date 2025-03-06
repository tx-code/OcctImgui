#define BOOST_TEST_MODULE Signal Tests
#include <boost/test/unit_test.hpp>

#include "mvvm/Signal.h"
#include <string>
#include <vector>

using namespace MVVM;

// Simple test case for basic Signal functionality
BOOST_AUTO_TEST_CASE(signal_basic_test)
{
    // Arrange
    Signal<int> signal;
    int callCount = 0;
    int lastValue = 0;
    
    // Act
    auto connection = signal.connect([&](int value) {
        callCount++;
        lastValue = value;
    });
    
    signal.emit(42);
    
    // Assert
    BOOST_CHECK_EQUAL(callCount, 1);
    BOOST_CHECK_EQUAL(lastValue, 42);
    BOOST_CHECK_EQUAL(signal.slotCount(), 1);
    
    // Act again - disconnect and verify no more calls
    connection.disconnect();
    signal.emit(100);
    
    // Assert
    BOOST_CHECK_EQUAL(callCount, 1); // Still 1, not incremented
    BOOST_CHECK_EQUAL(lastValue, 42); // Still 42, not updated
    BOOST_CHECK_EQUAL(signal.slotCount(), 0);
}

// Test multiple parameters
BOOST_AUTO_TEST_CASE(signal_multiple_params_test)
{
    // Arrange
    Signal<std::string, int> signal;
    std::string lastString;
    int lastInt = 0;
    
    // Act
    signal.connect([&](const std::string& str, int value) {
        lastString = str;
        lastInt = value;
    });
    
    signal.emit("hello", 123);
    
    // Assert
    BOOST_CHECK_EQUAL(lastString, "hello");
    BOOST_CHECK_EQUAL(lastInt, 123);
}

// Test multiple slots
BOOST_AUTO_TEST_CASE(signal_multiple_slots_test)
{
    // Arrange
    Signal<int> signal;
    std::vector<int> results;
    
    // Act
    signal.connect([&](int value) { results.push_back(value); });
    signal.connect([&](int value) { results.push_back(value * 2); });
    signal.connect([&](int value) { results.push_back(value * 3); });
    
    signal.emit(10);
    
    // Assert
    BOOST_CHECK_EQUAL(results.size(), 3);
    BOOST_CHECK_EQUAL(results[0], 10);
    BOOST_CHECK_EQUAL(results[1], 20);
    BOOST_CHECK_EQUAL(results[2], 30);
    BOOST_CHECK_EQUAL(signal.slotCount(), 3);
    
    // Act - disconnect all
    signal.disconnectAll();
    results.clear();
    signal.emit(5);
    
    // Assert
    BOOST_CHECK(results.empty());
    BOOST_CHECK_EQUAL(signal.slotCount(), 0);
}

// Test class with member function
class TestReceiver {
public:
    void onSignal(int value) {
        receivedValue = value;
        callCount++;
    }
    
    void onMultiParamSignal(const std::string& str, int value) {
        receivedString = str;
        receivedValue = value;
        callCount++;
    }
    
    int receivedValue = 0;
    std::string receivedString;
    int callCount = 0;
};

BOOST_AUTO_TEST_CASE(signal_member_function_test)
{
    // Arrange
    Signal<int> signal;
    TestReceiver receiver;
    
    // Act - use lambda instead of direct member function connection
    auto connection = signal.connect([&receiver](int value) {
        receiver.onSignal(value);
    });
    signal.emit(42);
    
    // Assert
    BOOST_CHECK_EQUAL(receiver.callCount, 1);
    BOOST_CHECK_EQUAL(receiver.receivedValue, 42);

    connection.disconnect();

    // Act - use direct member function connection
    auto connection2 = signal.connect(&receiver, &TestReceiver::onSignal);
    signal.emit(43);

    // Assert
    BOOST_CHECK_EQUAL(receiver.callCount, 2);
    BOOST_CHECK_EQUAL(receiver.receivedValue, 43);
    
    // Clean up
    connection2.disconnect();
}

// Test the operator()
BOOST_AUTO_TEST_CASE(signal_operator_test)
{
    Signal<int> signal;
    signal.connect([](int value) {
        BOOST_CHECK_EQUAL(value, 42);
    });
    
    signal(42);
}

// Test ScopedConnection
BOOST_AUTO_TEST_CASE(scoped_connection_test)
{
    // Arrange
    Signal<int> signal;
    int callCount = 0;
    
    // Act - create a scope
    {
        ScopedConnection connection(signal.connect([&](int) { callCount++; }));
        signal.emit(1);
        BOOST_CHECK_EQUAL(callCount, 1);
        BOOST_CHECK(connection.connected());
        
        // Connection should be automatically disconnected when leaving scope
    }
    
    // Assert - after scope exit
    signal.emit(2);
    BOOST_CHECK_EQUAL(callCount, 1); // Still 1, not incremented
    BOOST_CHECK_EQUAL(signal.slotCount(), 0);
}

// Test ScopedConnection move semantics
BOOST_AUTO_TEST_CASE(scoped_connection_move_test)
{
    // Arrange
    Signal<int> signal;
    int callCount = 0;
    
    // Act
    ScopedConnection connection1(signal.connect([&](int) { callCount++; }));
    ScopedConnection connection2 = std::move(connection1);
    
    signal.emit(1);
    
    // Assert
    BOOST_CHECK_EQUAL(callCount, 1);
    BOOST_CHECK(!connection1.connected()); // Moved from
    BOOST_CHECK(connection2.connected());  // Moved to
    
    // Disconnect and verify
    connection2.disconnect();
    signal.emit(2);
    BOOST_CHECK_EQUAL(callCount, 1); // Still 1, not incremented
}

// Test ConnectionTracker
BOOST_AUTO_TEST_CASE(connection_tracker_test)
{
    // Arrange
    Signal<int> signal1;
    Signal<std::string> signal2;
    int signal1CallCount = 0;
    int signal2CallCount = 0;
    ConnectionTracker tracker;
    
    // Act
    tracker.track(signal1.connect([&](int) { signal1CallCount++; }));
    tracker.track(signal2.connect([&](const std::string&) { signal2CallCount++; }));
    
    signal1.emit(1);
    signal2.emit("test");
    
    // Assert
    BOOST_CHECK_EQUAL(signal1CallCount, 1);
    BOOST_CHECK_EQUAL(signal2CallCount, 1);
    
    // Act - disconnect all
    tracker.disconnectAll();
    signal1.emit(2);
    signal2.emit("test2");
    
    // Assert
    BOOST_CHECK_EQUAL(signal1CallCount, 1); // Still 1, not incremented
    BOOST_CHECK_EQUAL(signal2CallCount, 1); // Still 1, not incremented
}

// Test ConnectionTracker with ScopedConnection
BOOST_AUTO_TEST_CASE(connection_tracker_with_scoped_connection_test)
{
    // Arrange
    Signal<int> signal;
    int callCount = 0;
    ConnectionTracker tracker;
    ScopedConnection connection(signal.connect([&](int) { callCount++; }));
    
    // Act
    tracker.track(connection);
    signal.emit(1);
    
    // Assert
    BOOST_CHECK_EQUAL(callCount, 1);
    
    // Act - disconnect all through tracker
    tracker.disconnectAll();
    signal.emit(2);
    
    // Assert
    BOOST_CHECK_EQUAL(callCount, 1); // Still 1, not incremented
    BOOST_CHECK(!connection.connected());
}

// Test ConnectionTracker with direct signal tracking
BOOST_AUTO_TEST_CASE(connection_tracker_direct_signal_tracking_test)
{
    // Arrange
    Signal<int> signal;
    int callCount = 0;
    ConnectionTracker tracker;
    
    // Act - use the track method that takes a signal and slot
    tracker.track(signal.connect([&](int) { callCount++; }));
    signal.emit(1);
    
    // Assert
    BOOST_CHECK_EQUAL(callCount, 1);
    
    // Act - disconnect all
    tracker.disconnectAll();
    signal.emit(2);
    
    // Assert
    BOOST_CHECK_EQUAL(callCount, 1); // Still 1, not incremented
}

// Test automatic cleanup when ConnectionTracker is destroyed
BOOST_AUTO_TEST_CASE(connection_tracker_destruction_test)
{
    // Arrange
    Signal<int> signal;
    int callCount = 0;
    
    // Act - create a scope with tracker
    {
        ConnectionTracker tracker;
        tracker.track(signal.connect([&](int) { callCount++; }));
        signal.emit(1);
        BOOST_CHECK_EQUAL(callCount, 1);
        
        // Tracker should disconnect all connections when destroyed
    }
    
    // Assert - after tracker destruction
    signal.emit(2);
    BOOST_CHECK_EQUAL(callCount, 1); // Still 1, not incremented
    BOOST_CHECK_EQUAL(signal.slotCount(), 0);
} 