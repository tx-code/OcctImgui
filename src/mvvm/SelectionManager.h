#pragma once

#include "mvvm/MessageBus.h"
#include <AIS_InteractiveObject.hxx>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace MVVM
{

class SelectionManager
{
public:
    // 获取单例实例
    static SelectionManager& getInstance();

    // 删除拷贝构造函数和赋值操作符
    SelectionManager(const SelectionManager&) = delete;
    SelectionManager& operator=(const SelectionManager&) = delete;

    // Selection methods
    void addToSelection(const Handle(AIS_InteractiveObject) & object, const std::string& objectId);
    void addToSelection(const Handle(AIS_InteractiveObject) & object,
                        const std::string& objectId,
                        const std::vector<SelectionInfo::SubFeatureIdentifier>& subFeatures);

    void removeFromSelection(const Handle(AIS_InteractiveObject) & object,
                             const std::string& objectId);
    void removeFromSelection(const std::string& objectId);

    void clearSelection();

    // Set selection mode
    void setSelectionMode(int mode);

    // Set selection type
    void setSelectionType(SelectionInfo::SelectionType type);

    // Get current selection
    const SelectionInfo& getCurrentSelection() const;

private:
    // 私有构造函数
    SelectionManager();

    // Notify selection changes through MessageBus
    void notifySelectionChanged();

    // MessageBus reference
    MessageBus& myMessageBus;

    // Current selection state
    SelectionInfo mySelectionInfo;
};

}  // namespace MVVM
