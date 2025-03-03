/**
 * @file OcctView.h
 * @brief Defines the OcctView class which provides 3D visualization using OpenCASCADE.
 * 
 * The OcctView class is responsible for rendering 3D content using the OpenCASCADE
 * Technology (OCCT) visualization framework. It handles user interactions with the
 * 3D view and communicates with the ViewModel layer.
 */
#pragma once

#include "../GlfwOcctWindow.h"
#include "../viewmodel/UnifiedViewModel.h"
#include "../mvvm/MessageBus.h"
#include "IView.h"
#include <AIS_ViewController.hxx>
#include <memory>

class AIS_ViewCube;

/**
 * @class OcctView
 * @brief View component for 3D visualization using OpenCASCADE.
 * 
 * This class implements the IView interface and extends AIS_ViewController to provide
 * 3D visualization capabilities. It renders the geometric data from the UnifiedViewModel
 * and handles user interactions with the 3D view.
 */
class OcctView: public IView, protected AIS_ViewController
{
public:
    /**
     * @brief Constructor
     * @param viewModel The UnifiedViewModel to connect with
     * @param window The GLFW OCCT window for rendering
     * @param messageBus Reference to the message bus for event communication
     */
    OcctView(std::shared_ptr<UnifiedViewModel> viewModel, 
             Handle(GlfwOcctWindow) window,
             MVVM::MessageBus& messageBus);
    
    /**
     * @brief Destructor
     */
    ~OcctView() override;

    /**
     * @brief Initializes the view with a GLFW window
     * @param window The GLFW window to initialize with
     */
    void initialize(GLFWwindow* window) override;
    
    /**
     * @brief Prepares for rendering a new frame
     */
    void newFrame() override;
    
    /**
     * @brief Renders the view
     */
    void render() override;
    
    /**
     * @brief Shuts down the view
     */
    void shutdown() override;
    
    /**
     * @brief Checks if the view wants to capture mouse input
     * @return True if the view wants to capture mouse input, false otherwise
     */
    bool wantCaptureMouse() const override;
    
    /**
     * @brief Gets the view model
     * @return Shared pointer to the view model
     */
    std::shared_ptr<IViewModel> getViewModel() const override;

    /**
     * @brief Initializes the view
     */
    void initialize();
    
    /**
     * @brief Cleans up resources
     */
    void cleanup();

    /**
     * @brief Handles mouse movement events
     * @param posX The x-coordinate of the mouse position
     * @param posY The y-coordinate of the mouse position
     */
    void onMouseMove(int posX, int posY);
    
    /**
     * @brief Handles mouse button events
     * @param button The mouse button that was pressed or released
     * @param action The action (press, release) that occurred
     * @param mods Modifier keys that were held down
     */
    void onMouseButton(int button, int action, int mods);
    
    /**
     * @brief Handles mouse scroll events
     * @param offsetX The horizontal scroll offset
     * @param offsetY The vertical scroll offset
     */
    void onMouseScroll(double offsetX, double offsetY);
    
    /**
     * @brief Handles window resize events
     * @param width The new width of the window
     * @param height The new height of the window
     */
    void onResize(int width, int height);

    /**
     * @brief Gets the OCCT view
     * @return The OCCT view
     */
    Handle(V3d_View) getView() const
    {
        return myView;
    }

    /**
     * @brief Checks if the view should wait for events
     * @return True if the view should wait for events, false otherwise
     */
    bool toWaitEvents() const
    {
        return myToWaitEvents;
    }

protected:
    /**
     * @brief Handles view redraw requests
     * @param theCtx The interactive context
     * @param theView The view to redraw
     */
    void handleViewRedraw(const Handle(AIS_InteractiveContext) & theCtx,
                          const Handle(V3d_View) & theView) override;

private:
    /** The view model */
    std::shared_ptr<UnifiedViewModel> myViewModel;
    
    /** The GLFW OCCT window */
    Handle(GlfwOcctWindow) myWindow;
    
    /** The OCCT view */
    Handle(V3d_View) myView;
    
    /** The view cube for orientation */
    Handle(AIS_ViewCube) myViewCube;
    
    /** Reference to the message bus */
    MVVM::MessageBus& myMessageBus;
    
    /** Flag indicating whether to wait for events */
    bool myToWaitEvents = true;

    /**
     * @brief Sets up the view cube
     */
    void setupViewCube();
    
    /**
     * @brief Sets up the grid
     */
    void setupGrid();
    
    /**
     * @brief Updates visibility of elements
     */
    void updateVisibility();

    /**
     * @brief Handles selection at a specific position
     * @param x The x-coordinate of the selection position
     * @param y The y-coordinate of the selection position
     */
    void handleSelection(int x, int y);

    /**
     * @brief Subscribes to events from the message bus
     */
    void subscribeToEvents();
};