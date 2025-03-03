#pragma once

#include "Observable.h"
#include <memory>

namespace MVVM {

// 全局设置单例类，用于存储全局UI设置
class GlobalSettings {
public:
    // 获取单例实例
    static GlobalSettings& getInstance() {
        static GlobalSettings instance;
        return instance;
    }

    // 禁止拷贝和移动
    GlobalSettings(const GlobalSettings&) = delete;
    GlobalSettings& operator=(const GlobalSettings&) = delete;
    GlobalSettings(GlobalSettings&&) = delete;
    GlobalSettings& operator=(GlobalSettings&&) = delete;

    // 全局UI设置
    Observable<bool> isGridVisible{true};
    Observable<bool> isViewCubeVisible{true};

private:
    // 私有构造函数确保单例
    GlobalSettings() = default;
    ~GlobalSettings() = default;
};

} // namespace MVVM 