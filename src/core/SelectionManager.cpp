#include "SelectionManager.h"
#include "utils/Logger.h"
#include <algorithm>


using namespace Core;

// 使用宏声明 SelectionManager 类的 logger
DECLARE_LOGGER(SelectionManager)

SelectionManager& SelectionManager::getInstance()
{
    static SelectionManager instance;
    return instance;
}

SelectionManager::SelectionManager()
    : myMessageBus(MessageBus::getInstance())
{
    // Initialize selection info
    mySelectionInfo.selectionMode = 0;
    mySelectionInfo.selectionType = SelectionInfo::SelectionType::New;
    getSelectionManagerLogger()->info("SelectionManager initialized with New selection type");
}

void SelectionManager::addToSelection(const Handle(AIS_InteractiveObject) & object,
                                      const std::string& objectId)
{
    auto logger = getSelectionManagerLogger();
    logger->info("Adding object {} to selection (type: {})",
                 objectId,
                 mySelectionInfo.selectionType == SelectionInfo::SelectionType::New ? "New"
                     : mySelectionInfo.selectionType == SelectionInfo::SelectionType::Add
                     ? "Add"
                     : "Remove");

    // Clear previous selection if type is New
    if (mySelectionInfo.selectionType == SelectionInfo::SelectionType::New) {
        mySelectionInfo.selectedObjects.clear();
        mySelectionInfo.subFeatures.clear();
        logger->debug("Cleared previous selection (New type)");
    }

    // Add to selection if not removing
    if (mySelectionInfo.selectionType != SelectionInfo::SelectionType::Remove) {
        // Check if object is already in selection
        auto it = std::find(mySelectionInfo.selectedObjects.begin(),
                            mySelectionInfo.selectedObjects.end(),
                            object);

        if (it == mySelectionInfo.selectedObjects.end()) {
            mySelectionInfo.selectedObjects.push_back(object);
            logger->debug("Added object {} to selection. Current selection size: {}",
                          objectId,
                          mySelectionInfo.selectedObjects.size());
        }
        else {
            logger->debug("Object {} already in selection", objectId);
        }
    }
    // Remove from selection if type is Remove
    else {
        auto it = std::find(mySelectionInfo.selectedObjects.begin(),
                            mySelectionInfo.selectedObjects.end(),
                            object);

        if (it != mySelectionInfo.selectedObjects.end()) {
            mySelectionInfo.selectedObjects.erase(it);
            mySelectionInfo.subFeatures.erase(objectId);
            logger->debug("Removed object {} from selection. Current selection size: {}",
                          objectId,
                          mySelectionInfo.selectedObjects.size());
        }
        else {
            logger->debug("Object {} not found in selection for removal", objectId);
        }
    }

    // Notify selection changed
    notifySelectionChanged();
}

void SelectionManager::addToSelection(
    const Handle(AIS_InteractiveObject) & object,
    const std::string& objectId,
    const std::vector<SelectionInfo::SubFeatureIdentifier>& subFeatures)
{
    auto logger = getSelectionManagerLogger();
    logger->info("Adding object {} with {} subfeatures to selection", objectId, subFeatures.size());

    // First add the object to selection
    addToSelection(object, objectId);

    // Add subfeature information
    if (mySelectionInfo.selectionType != SelectionInfo::SelectionType::Remove) {
        mySelectionInfo.subFeatures[objectId] = subFeatures;
        logger->debug("Added {} subfeatures for object {}", subFeatures.size(), objectId);

        // Log subfeature details
        for (const auto& subFeature : subFeatures) {
            logger->debug("  - Type: {}, Index: {}",
                          subFeature.type == SelectionInfo::SubFeatureType::Face       ? "Face"
                              : subFeature.type == SelectionInfo::SubFeatureType::Edge ? "Edge"
                                                                                       : "Vertex",
                          subFeature.index);
        }
    }

    // Notify selection changed
    notifySelectionChanged();
}

void SelectionManager::removeFromSelection(const Handle(AIS_InteractiveObject) & object,
                                           const std::string& objectId)
{
    auto logger = getSelectionManagerLogger();
    logger->info("Removing object {} from selection", objectId);

    auto it = std::find(mySelectionInfo.selectedObjects.begin(),
                        mySelectionInfo.selectedObjects.end(),
                        object);

    if (it != mySelectionInfo.selectedObjects.end()) {
        mySelectionInfo.selectedObjects.erase(it);
        mySelectionInfo.subFeatures.erase(objectId);
        logger->debug("Removed object {} and its subfeatures. Current selection size: {}",
                      objectId,
                      mySelectionInfo.selectedObjects.size());

        // Notify selection changed
        notifySelectionChanged();
    }
    else {
        logger->debug("Object {} not found in selection for removal", objectId);
    }
}

void SelectionManager::removeFromSelection(const std::string& objectId)
{
    auto logger = getSelectionManagerLogger();
    logger->info("Removing object {} from selection by ID", objectId);

    // Find and remove the object with the given ID
    auto subFeaturesIt = mySelectionInfo.subFeatures.find(objectId);
    if (subFeaturesIt != mySelectionInfo.subFeatures.end()) {
        // Remove from subFeatures
        mySelectionInfo.subFeatures.erase(subFeaturesIt);
        logger->debug("Removed subfeatures for object {}", objectId);

        // Remove from selectedObjects
        auto it = mySelectionInfo.selectedObjects.begin();
        while (it != mySelectionInfo.selectedObjects.end()) {
            // Here we would need to match the object to its ID
            // For simplicity, we'll remove first occurrence
            it = mySelectionInfo.selectedObjects.erase(it);
            logger->debug("Removed object from selection. Current selection size: {}",
                          mySelectionInfo.selectedObjects.size());

            // Notify selection changed
            notifySelectionChanged();
            break;
        }
    }
    else {
        logger->debug("Object {} not found in selection for removal", objectId);
    }
}

void SelectionManager::clearSelection()
{
    auto logger = getSelectionManagerLogger();
    logger->info("Clearing all selections");

    mySelectionInfo.selectedObjects.clear();
    mySelectionInfo.subFeatures.clear();
    logger->debug("Cleared {} objects and {} subfeature entries",
                  mySelectionInfo.selectedObjects.size(),
                  mySelectionInfo.subFeatures.size());

    // Notify selection changed
    notifySelectionChanged();
}

void SelectionManager::setSelectionMode(int mode)
{
    auto logger = getSelectionManagerLogger();
    logger->info("Setting selection mode to {}", mode);
    mySelectionInfo.selectionMode = mode;
}

void SelectionManager::setSelectionType(SelectionInfo::SelectionType type)
{
    auto logger = getSelectionManagerLogger();
    logger->info("Setting selection type to {}",
                 type == SelectionInfo::SelectionType::New       ? "New"
                     : type == SelectionInfo::SelectionType::Add ? "Add"
                                                                 : "Remove");
    mySelectionInfo.selectionType = type;
}

const SelectionInfo& SelectionManager::getCurrentSelection() const
{
    return mySelectionInfo;
}

bool SelectionManager::hasSelection() const
{
    return !mySelectionInfo.selectedObjects.empty();
}

void SelectionManager::notifySelectionChanged()
{
    auto logger = getSelectionManagerLogger();
    logger->debug("Notifying selection changed: {} objects, {} subfeature entries",
                  mySelectionInfo.selectedObjects.size(),
                  mySelectionInfo.subFeatures.size());

    // Create a message with the selection info
    MessageBus::Message message;
    message.type = MessageBus::MessageType::SelectionChanged;
    message.data = mySelectionInfo;

    // Publish the message
    myMessageBus.publish(message);
}