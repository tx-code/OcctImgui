#pragma once

#include "Property.h"
#include <memory>

namespace MVVM {

/**
 * @class GlobalSettings
 * @brief Global application settings using the Property system
 * 
 * This class stores global UI settings and other application-wide settings
 * using the Property system for change notification and binding.
 */
class GlobalSettings {
public:
    // Constructor
    GlobalSettings() = default;
    ~GlobalSettings() = default;

    // Disable copy and move
    GlobalSettings(const GlobalSettings&) = delete;
    GlobalSettings& operator=(const GlobalSettings&) = delete;
    GlobalSettings(GlobalSettings&&) = delete;
    GlobalSettings& operator=(GlobalSettings&&) = delete;

    // UI settings
    Property<bool> isGridVisible{true};
    Property<bool> isViewCubeVisible{true};
    
    // Display settings
    Property<int> displayMode{0}; // 0: Shaded, 1: Wireframe, 2: Vertices, etc.
    
    // View settings
    Property<double> cameraDistance{100.0};
    Property<bool> perspectiveMode{true};
    
    // Selection settings
    Property<bool> highlightOnHover{true};
    
    // Connection tracker for property bindings
    ConnectionTracker connections;
};

} // namespace MVVM 