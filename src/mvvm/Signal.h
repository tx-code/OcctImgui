#pragma once

#include <boost/signals2.hpp>
#include <functional>
#include <memory>
#include "../utils/Logger.h"

namespace MVVM {

// Create MVVM logger - use function to ensure safe initialization
inline std::shared_ptr<Utils::Logger>& getSignalLogger() {
    static std::shared_ptr<Utils::Logger> logger = Utils::Logger::getLogger("signal");
    return logger;
}

/**
 * @class Signal
 * @brief A wrapper around boost::signals2 that provides a simpler interface for the MVVM pattern
 * 
 * This class provides a type-safe signal implementation that can be used to connect
 * components in the MVVM architecture. It supports automatic connection management
 * and thread safety.
 */
template<typename... Args>
class Signal {
public:
    using SlotType = std::function<void(Args...)>;
    using ConnectionType = boost::signals2::connection;
    using SignalType = boost::signals2::signal<void(Args...)>;
    
    /**
     * @brief Connect a slot to this signal
     * @param slot The function to call when the signal is emitted
     * @return A connection object that can be used to disconnect the slot
     */
    ConnectionType connect(const SlotType& slot) {
        getSignalLogger()->debug("Signal: Connecting new slot");
        return mySignal.connect(slot);
    }
    
    /**
     * @brief Connect a member function of an object to this signal
     * @param object The object instance
     * @param method The member function to call
     * @return A connection object that can be used to disconnect the slot
     */
    template<typename T>
    ConnectionType connect(T* object, void (T::*method)(Args...)) {
        getSignalLogger()->debug("Signal: Connecting member function");
        return mySignal.connect(std::bind(method, object, std::placeholders::_1, std::placeholders::_2));
    }
    
    /**
     * @brief Emit the signal, calling all connected slots
     * @param args The arguments to pass to the slots
     */
    void emit(Args... args) {
        getSignalLogger()->debug("Signal: Emitting signal to {} slots", mySignal.num_slots());
        mySignal(args...);
    }
    
    /**
     * @brief Disconnect all slots from this signal
     */
    void disconnectAll() {
        getSignalLogger()->debug("Signal: Disconnecting all slots");
        mySignal.disconnect_all_slots();
    }
    
    /**
     * @brief Get the number of connected slots
     * @return The number of slots
     */
    std::size_t slotCount() const {
        return mySignal.num_slots();
    }
    
private:
    SignalType mySignal;
};

// Forward declaration
class ConnectionTracker;

/**
 * @class ScopedConnection
 * @brief RAII wrapper for a boost::signals2 connection
 * 
 * This class automatically disconnects the connection when it goes out of scope.
 */
class ScopedConnection {
public:
    ScopedConnection() = default;
    
    explicit ScopedConnection(boost::signals2::connection connection)
        : myConnection(connection) {}
    
    ~ScopedConnection() {
        disconnect();
    }
    
    ScopedConnection(const ScopedConnection&) = delete;
    ScopedConnection& operator=(const ScopedConnection&) = delete;
    
    ScopedConnection(ScopedConnection&& other) noexcept
        : myConnection(std::move(other.myConnection)) {}
    
    ScopedConnection& operator=(ScopedConnection&& other) noexcept {
        if (this != &other) {
            disconnect();
            myConnection = std::move(other.myConnection);
        }
        return *this;
    }
    
    void disconnect() {
        if (myConnection.connected()) {
            myConnection.disconnect();
        }
    }
    
    bool connected() const {
        return myConnection.connected();
    }
    
    // Allow ConnectionTracker to access the connection
    friend class ConnectionTracker;
    
private:
    boost::signals2::connection myConnection;
};

/**
 * @class ConnectionTracker
 * @brief Tracks multiple connections and disconnects them when destroyed
 * 
 * This class is useful for objects that need to track multiple connections
 * and ensure they are all disconnected when the object is destroyed.
 */
class ConnectionTracker {
public:
    ConnectionTracker() = default;
    ~ConnectionTracker() {
        disconnectAll();
    }
    
    ConnectionTracker(const ConnectionTracker&) = delete;
    ConnectionTracker& operator=(const ConnectionTracker&) = delete;
    
    void track(boost::signals2::connection connection) {
        myConnections.push_back(connection);
    }
    
    void track(const ScopedConnection& connection) {
        if (connection.connected()) {
            myConnections.push_back(connection.myConnection);
        }
    }
    
    template<typename... Args>
    void track(Signal<Args...>& signal, const std::function<void(Args...)>& slot) {
        track(signal.connect(slot));
    }
    
    void disconnectAll() {
        for (auto& connection : myConnections) {
            if (connection.connected()) {
                connection.disconnect();
            }
        }
        myConnections.clear();
    }
    
private:
    std::vector<boost::signals2::connection> myConnections;
};

} // namespace MVVM 