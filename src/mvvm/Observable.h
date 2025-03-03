#pragma once

#include <functional>
#include <vector>
#include "../utils/Logger.h"

namespace MVVM {

// 创建MVVM日志记录器 - 使用函数确保安全初始化
inline std::shared_ptr<Utils::Logger>& getMvvmLogger() {
    static std::shared_ptr<Utils::Logger> logger = Utils::Logger::getLogger("mvvm");
    return logger;
}

template<typename T>
class Observable {
public:
    Observable() = default;
    Observable(const T& initialValue) : myValue(initialValue) {}
    
    void set(const T& newValue) {
        if (myValue != newValue) {
            myValue = newValue;
            notifyObservers();
        }
    }
    
    Observable& operator=(const T& newValue) {
        set(newValue);
        return *this;
    }
    
    const T& get() const { return myValue; }
    
    void addObserver(std::function<void(const T&)> observer) {
        myObservers.push_back(observer);
    }
    
private:
    T myValue;
    std::vector<std::function<void(const T&)>> myObservers;
    
    void notifyObservers() {
        getMvvmLogger()->debug("Observable: Notifying {} observers", myObservers.size());
        for (auto& observer : myObservers) {
            observer(myValue);
        }
    }
};

} // namespace MVVM 