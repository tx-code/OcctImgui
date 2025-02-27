// MIT License
//
// Copyright(c) 2023 Shing Liu
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "GlfwOcctWindow.h"
#include "gui/ModelControlGui.h"
#include "gui/ModelTreeGui.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_ViewController.hxx>
#include <V3d_View.hxx>
#include <memory>

class ModelManager;

//! Sample class using GLFW for window creation.
class GlfwOcctView : public AIS_ViewController
{
public:
    //! Main constructor.
    GlfwOcctView();

    //! Destructor.
    virtual ~GlfwOcctView();

    //! Run application.
    void run();

private:
    //! Perform application initialization.
    void initWindow(int theWidth, int theHeight, const char* theTitle);

    //! Initialize OCCT viewer.
    void initViewer();

    //! Initialize GUI elements.
    void initGui();

    //! Create some demonstration shapes.
    void initDemoScene();

    //! Main application event loop.
    void mainloop();

    //! Clean up resources.
    void cleanup();

    //! Render GUI.
    void renderGui();

    //! Handle view redraw.
    void handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx,
                          const Handle(V3d_View)& theView) override;

private:
    //! GLFW callbacks.
    static void errorCallback(int theError, const char* theDescription);
    static GlfwOcctView* toView(GLFWwindow* theWin);

    //! Window resize event.
    void onResize(int theWidth, int theHeight);
    static void onResizeCallback(GLFWwindow* theWin, int theWidth, int theHeight)
    {
        toView(theWin)->onResize(theWidth, theHeight);
    }
    static void onFBResizeCallback(GLFWwindow* theWin, int theWidth, int theHeight)
    {
        toView(theWin)->onResize(theWidth, theHeight);
    }

    //! Mouse scroll event.
    void onMouseScroll(double theOffsetX, double theOffsetY);
    static void onMouseScrollCallback(GLFWwindow* theWin, double theOffsetX, double theOffsetY)
    {
        toView(theWin)->onMouseScroll(theOffsetX, theOffsetY);
    }

    //! Mouse click event.
    void onMouseButton(int theButton, int theAction, int theMods);
    static void onMouseButtonCallback(GLFWwindow* theWin, int theButton, int theAction, int theMods)
    {
        toView(theWin)->onMouseButton(theButton, theAction, theMods);
    }

    //! Mouse move event.
    void onMouseMove(int thePosX, int thePosY);
    static void onMouseMoveCallback(GLFWwindow* theWin, double thePosX, double thePosY)
    {
        toView(theWin)->onMouseMove(static_cast<int>(thePosX), static_cast<int>(thePosY));
    }

private:
    Handle(GlfwOcctWindow) myOcctWindow;
    Handle(V3d_View) myView;
    Handle(AIS_InteractiveContext) myContext;
    bool myToWaitEvents = true;

    // GUI组件
    std::shared_ptr<ModelManager> myModelManager;
    std::shared_ptr<ModelTreeGui> myModelTree;
    std::shared_ptr<ModelControlGui> myModelControl;
};
