#define BOOST_TEST_MODULE SelectionManager Tests
#include <boost/test/unit_test.hpp>

#include "mvvm/MessageBus.h"
#include "mvvm/SelectionManager.h"
#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <Standard_Handle.hxx>
#include <TopoDS_Shape.hxx>
#include <memory>
#include <string>
#include <vector>

using namespace MVVM;

// 基本功能测试
BOOST_AUTO_TEST_CASE(selection_manager_basic_test)
{
    // 准备
    auto& messageBus = MessageBus::getInstance();
    auto& selectionManager = SelectionManager::getInstance();

    // 跟踪消息发布
    bool messageReceived = false;
    SelectionInfo receivedInfo;

    // 订阅SelectionChanged消息
    messageBus.subscribe(MessageBus::MessageType::SelectionChanged,
                         [&](const MessageBus::Message& message) {
                             messageReceived = true;
                             if (message.data.type() == typeid(SelectionInfo)) {
                                 receivedInfo = std::any_cast<SelectionInfo>(message.data);
                             }
                         });

    // 创建模拟对象
    Handle(AIS_Shape) mockObject = new AIS_Shape(TopoDS_Shape());
    std::string objectId = "TestObject1";

    // 测试添加到选择
    selectionManager.addToSelection(mockObject, objectId);

    // 断言
    BOOST_CHECK(messageReceived);
    BOOST_CHECK_EQUAL(receivedInfo.selectedObjects.size(), 1);

    messageBus.clearAllSubscriptions();
    selectionManager.clearSelection();
}

// 测试添加和移除选择
BOOST_AUTO_TEST_CASE(selection_manager_add_remove_test)
{
    // 准备
    auto& messageBus = MessageBus::getInstance();
    auto& selectionManager = SelectionManager::getInstance();

    // 设置选择类型为Add，这样可以添加多个对象而不清除之前的选择
    selectionManager.setSelectionType(SelectionInfo::SelectionType::Add);

    // 创建模拟对象
    Handle(AIS_Shape) mockObject1 = new AIS_Shape(TopoDS_Shape());
    Handle(AIS_Shape) mockObject2 = new AIS_Shape(TopoDS_Shape());
    std::string objectId1 = "TestObject1";
    std::string objectId2 = "TestObject2";

    // 添加两个对象
    selectionManager.addToSelection(mockObject1, objectId1);
    selectionManager.addToSelection(mockObject2, objectId2);

    // 验证当前选择
    {
        const auto& selection = selectionManager.getCurrentSelection();
        BOOST_CHECK_EQUAL(selection.selectedObjects.size(), 2);
    }

    // 移除一个对象
    selectionManager.removeFromSelection(mockObject1, objectId1);

    // 验证当前选择
    {
        const auto& selection = selectionManager.getCurrentSelection();
        BOOST_CHECK_EQUAL(selection.selectedObjects.size(), 1);
    }

    // 清除所有选择
    selectionManager.clearSelection();

    // 验证当前选择
    {
        const auto& selection = selectionManager.getCurrentSelection();
        BOOST_CHECK_EQUAL(selection.selectedObjects.size(), 0);
    }
}

// 测试子特征信息
BOOST_AUTO_TEST_CASE(selection_manager_subfeature_test)
{
    // 准备
    auto& messageBus = MessageBus::getInstance();
    auto& selectionManager = SelectionManager::getInstance();

    // 创建模拟对象
    Handle(AIS_Shape) mockObject = new AIS_Shape(TopoDS_Shape());
    std::string objectId = "TestObject1";

    // 创建子特征
    std::vector<SelectionInfo::SubFeatureIdentifier> subFeatures;
    subFeatures.emplace_back(SelectionInfo::SubFeatureType::Face, 1);
    subFeatures.emplace_back(SelectionInfo::SubFeatureType::Edge, 3);

    // 添加带子特征的对象
    selectionManager.addToSelection(mockObject, objectId, subFeatures);

    // 验证当前选择
    {
        const auto& selection = selectionManager.getCurrentSelection();
        BOOST_CHECK_EQUAL(selection.selectedObjects.size(), 1);
        BOOST_CHECK_EQUAL(selection.subFeatures.size(), 1);

        auto it = selection.subFeatures.find(objectId);
        BOOST_CHECK(it != selection.subFeatures.end());
        BOOST_CHECK_EQUAL(it->second.size(), 2);
        BOOST_CHECK_EQUAL(it->second[0].type, SelectionInfo::SubFeatureType::Face);
        BOOST_CHECK_EQUAL(it->second[0].index, 1);
        BOOST_CHECK_EQUAL(it->second[1].type, SelectionInfo::SubFeatureType::Edge);
        BOOST_CHECK_EQUAL(it->second[1].index, 3);
    }

    selectionManager.clearSelection();
}

// 测试选择模式和类型
BOOST_AUTO_TEST_CASE(selection_manager_mode_type_test)
{
    // 准备
    auto& messageBus = MessageBus::getInstance();
    auto& selectionManager = SelectionManager::getInstance();

    // 设置选择模式
    int testMode = 2;
    selectionManager.setSelectionMode(testMode);

    // 验证选择模式
    {
        const auto& selection = selectionManager.getCurrentSelection();
        BOOST_CHECK_EQUAL(selection.selectionMode, testMode);
    }

    // 设置选择类型
    selectionManager.setSelectionType(SelectionInfo::SelectionType::Add);

    // 验证选择类型
    {
        const auto& selection = selectionManager.getCurrentSelection();
        BOOST_CHECK_EQUAL(selection.selectionType, SelectionInfo::SelectionType::Add);
    }

    selectionManager.clearSelection();
}