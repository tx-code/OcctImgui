#pragma once

#include <memory>
#include <string>
#include <map>
#include <functional>
#include "IModel.h"

class ModelFactory {
public:
    static ModelFactory& instance() {
        static ModelFactory factory;
        return factory;
    }
    
    template<typename T>
    void registerModelType(const std::string& typeName) {
        myModelCreators[typeName] = []() { return std::make_shared<T>(); };
    }
    
    std::shared_ptr<IModel> createModel(const std::string& typeName) {
        auto it = myModelCreators.find(typeName);
        if (it != myModelCreators.end()) {
            return it->second();
        }
        return nullptr;
    }
    
private:
    ModelFactory() = default;
    ~ModelFactory() = default;
    
    std::map<std::string, std::function<std::shared_ptr<IModel>()>> myModelCreators;
}; 