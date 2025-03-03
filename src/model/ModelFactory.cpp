#include "ModelFactory.h"
#include "UnifiedModel.h"

// 初始化工厂
void InitializeModelFactory(ModelFactory& factory) {
    // 注册统一模型
    factory.registerModelType<UnifiedModel>("UnifiedModel");
} 