#pragma once

#include "../viewmodel/IViewModel.h"
#include <memory>

struct GLFWwindow;

class IView {
public:
    virtual ~IView() = default;
    
    virtual void initialize(GLFWwindow* window) = 0;
    virtual void newFrame() = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;
    
    virtual bool wantCaptureMouse() const = 0;
    
    // 访问器
    virtual std::shared_ptr<IViewModel> getViewModel() const = 0;
}; 