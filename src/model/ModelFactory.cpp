#include "ModelFactory.h"
#include "UnifiedModel.h"

// 初始化工厂
void InitializeModelFactory() {
    auto& factory = ModelFactory::instance();
    
    // 注册统一模型
    factory.registerModelType<UnifiedModel>("UnifiedModel");
} 