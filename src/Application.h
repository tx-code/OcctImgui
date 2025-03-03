#pragma once

#include "GlfwOcctWindow.h"
#include "model/UnifiedModel.h"
#include "viewmodel/UnifiedViewModel.h"
#include "view/ImGuiView.h"
#include "view/OcctView.h"

#include <memory>

class Application {
public:
    Application();
    ~Application();
    
    void run();
    
private:
    // 初始化方法
    void initWindow();
    void initModel();
    void initViewModel();
    void initViews();
    
    // 事件回调
    static void onResizeCallback(GLFWwindow* theWin, int theWidth, int theHeight);
    static void onFBResizeCallback(GLFWwindow* theWin, int theWidth, int theHeight);
    static void onMouseScrollCallback(GLFWwindow* theWin, double theOffsetX, double theOffsetY);
    static void onMouseButtonCallback(GLFWwindow* theWin, int theButton, int theAction, int theMods);
    static void onMouseMoveCallback(GLFWwindow* theWin, double thePosX, double thePosY);
    static void errorCallback(int theError, const char* theDescription);
    
    // 获取应用实例
    static Application* toApplication(GLFWwindow* theWin);
    
    // 主循环
    void mainloop();
    void cleanup();
    
    // MVVM组件
    std::shared_ptr<UnifiedModel> myModel;
    std::shared_ptr<UnifiedViewModel> myViewModel;
    std::unique_ptr<ImGuiView> myImGuiView;
    std::unique_ptr<OcctView> myOcctView;
    
    // 窗口管理
    Handle(GlfwOcctWindow) myWindow;
    GLFWwindow* myGlfwWindow;
    
    // 配置
    int myWidth;
    int myHeight;
    TCollection_AsciiString myTitle;
}; 