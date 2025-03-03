#pragma once

#include "Observable.h"
#include <memory>

namespace MVVM {

// 全局设置类，用于存储全局UI设置
class GlobalSettings {
public:
    // 构造函数
    GlobalSettings() = default;
    ~GlobalSettings() = default;

    // 禁止拷贝和移动
    GlobalSettings(const GlobalSettings&) = delete;
    GlobalSettings& operator=(const GlobalSettings&) = delete;
    GlobalSettings(GlobalSettings&&) = delete;
    GlobalSettings& operator=(GlobalSettings&&) = delete;

    // 全局UI设置
    Observable<bool> isGridVisible{true};
    Observable<bool> isViewCubeVisible{true};
};

} // namespace MVVM 