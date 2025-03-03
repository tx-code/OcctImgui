#pragma once

#include "Signal.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>
#include <string>
#include <memory>
#include <type_traits>
#include <any>

namespace MVVM {

/**
 * @class Property
 * @brief A property class that combines value storage with change notification
 * 
 * This class provides a property implementation that stores a value and notifies
 * observers when the value changes. It uses the Signal class for change notification
 * and provides a clean interface for property binding.
 */
template<typename T>
class Property {
public:
    // Signal types for property changes
    using ValueChangedSignal = Signal<const T&, const T&>; // (oldValue, newValue)
    
    /**
     * @brief Default constructor
     */
    Property() = default;
    
    /**
     * @brief Constructor with initial value
     * @param initialValue The initial value of the property
     */
    explicit Property(const T& initialValue) : myValue(initialValue) {}
    
    /**
     * @brief Get the current value
     * @return The current value
     */
    const T& get() const { return myValue; }
    
    /**
     * @brief Set a new value
     * @param newValue The new value to set
     * @return True if the value changed, false otherwise
     */
    bool set(const T& newValue) {
        if (myValue != newValue) {
            T oldValue = myValue;
            myValue = newValue;
            valueChanged.emit(oldValue, myValue);
            return true;
        }
        return false;
    }
    
    /**
     * @brief Assignment operator
     * @param newValue The new value to assign
     * @return Reference to this property
     */
    Property& operator=(const T& newValue) {
        set(newValue);
        return *this;
    }
    
    /**
     * @brief Conversion operator to the value type
     * @return The current value
     */
    operator const T&() const { return get(); }
    
    /**
     * @brief Bind this property to another property
     * @param other The property to bind to
     * @return A connection object that can be used to unbind
     */
    ScopedConnection bindTo(Property<T>& other) {
        return ScopedConnection(other.valueChanged.connect(
            [this](const T&, const T& newValue) {
                this->set(newValue);
            }
        ));
    }
    
    /**
     * @brief Bind this property to a function that computes its value
     * @param computeFunc The function that computes the value
     * @param dependencies The properties that this binding depends on
     * @return A vector of connection objects
     */
    template<typename... Props>
    std::vector<ScopedConnection> bindComputed(
        std::function<T(const Props&...)> computeFunc,
        Props&... dependencies) {
        
        std::vector<ScopedConnection> connections;
        
        // Helper to update the value from all dependencies
        auto updateValue = [this, computeFunc, &dependencies...]() {
            this->set(computeFunc(dependencies.get()...));
        };
        
        // Initial update
        updateValue();
        
        // Connect to each dependency
        (connections.push_back(ScopedConnection(
            dependencies.valueChanged.connect([updateValue](const auto&, const auto&) {
                updateValue();
            })
        )), ...);
        
        return connections;
    }
    
    /**
     * @brief Signal emitted when the value changes
     * 
     * The signal provides both the old and new values.
     */
    ValueChangedSignal valueChanged;
    
private:
    T myValue{};
};

/**
 * @class PropertyGroup
 * @brief A group of properties that can be managed together
 * 
 * This class provides a way to group related properties and manage their
 * connections together. It uses boost::property_tree for hierarchical property
 * organization.
 */
class PropertyGroup {
public:
    PropertyGroup() = default;
    virtual ~PropertyGroup() = default;
    
    /**
     * @brief Get a property by path
     * @param path The path to the property
     * @return The property value or boost::none if not found
     */
    template<typename T>
    boost::optional<T> getProperty(const std::string& path) const {
        try {
            return myProperties.get<T>(path);
        } catch (...) {
            return boost::none;
        }
    }
    
    /**
     * @brief Set a property value
     * @param path The path to the property
     * @param value The value to set
     */
    template<typename T>
    void setProperty(const std::string& path, const T& value) {
        T oldValue = myProperties.get<T>(path, T{});
        if (oldValue != value) {
            myProperties.put(path, value);
            propertyChanged.emit(path, oldValue, value);
        }
    }
    
    /**
     * @brief Check if a property exists
     * @param path The path to check
     * @return True if the property exists, false otherwise
     */
    bool hasProperty(const std::string& path) const {
        return !myProperties.get_child_optional(path).is_initialized();
    }
    
    /**
     * @brief Signal emitted when any property changes
     * 
     * The signal provides the path, old value, and new value.
     */
    Signal<std::string, std::any, std::any> propertyChanged;
    
private:
    boost::property_tree::ptree myProperties;
};

} // namespace MVVM 