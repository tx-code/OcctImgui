#pragma once

#include <AIS_InteractiveObject.hxx>
#include <any>
#include <map>
#include <ostream>
#include <vector>

namespace Core
{

// 选择信息结构，用于传递选中对象及其子特征
struct SelectionInfo
{
    // 子特征类型枚举
    enum class SubFeatureType
    {
        Face,   // TopoDS_Face or facet
        Edge,   // TopoDS_Edge or link
        Vertex  // TopoDS_Vertex or node
    };

    // 子特征标识符结构
    struct SubFeatureIdentifier
    {
        SubFeatureType type;
        int index;

        // 可选：额外数据（如参数坐标等）
        std::any additionalData;

        // 构造函数
        SubFeatureIdentifier(SubFeatureType t, int idx)
            : type(t)
            , index(idx)
        {}

        // 带额外数据的构造函数
        template<typename T>
        SubFeatureIdentifier(SubFeatureType t, int idx, const T& data)
            : type(t)
            , index(idx)
            , additionalData(data)
        {}
    };

    // 主要选中的交互对象
    std::vector<Handle(AIS_InteractiveObject)> selectedObjects;

    // 对象ID到选中子特征的映射
    std::map<std::string, std::vector<SubFeatureIdentifier>> subFeatures;

    // 当前活动的选择模式
    int selectionMode = 0;

    // 选择操作类型（新选择、添加选择、移除选择）
    enum class SelectionType
    {
        New,    // 新选择，替换之前的选择
        Add,    // 添加到现有选择
        Remove  // 从现有选择中移除
    } selectionType = SelectionType::New;
};


// 为 SelectionType 枚举添加输出运算符
inline std::ostream& operator<<(std::ostream& os, const SelectionInfo::SelectionType& type)
{
    switch (type) {
        // 替换为您的实际枚举值
        case SelectionInfo::SelectionType::New:
            return os << "New";
        case SelectionInfo::SelectionType::Add:
            return os << "Add";
        case SelectionInfo::SelectionType::Remove:
            return os << "Remove";
        // 添加其他枚举值...
        default:
            return os << "Unknown(" << static_cast<int>(type) << ")";
    }
}

// 为 SubFeatureType 枚举添加输出运算符
inline std::ostream& operator<<(std::ostream& os, const SelectionInfo::SubFeatureType& type)
{
    switch (type) {
        case SelectionInfo::SubFeatureType::Face:
            return os << "Face";
        case SelectionInfo::SubFeatureType::Edge:
            return os << "Edge";
        case SelectionInfo::SubFeatureType::Vertex:
            return os << "Vertex";
        default:
            return os << "Unknown(" << static_cast<int>(type) << ")";
    }
}

}  // namespace Core
