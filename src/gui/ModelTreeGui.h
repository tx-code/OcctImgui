#pragma once

#include "IGUIComponent.h"
#include <map>
#include <memory>
#include <string>

class ModelManager;

class ModelTreeGui : public IGUIComponent {
public:
    ModelTreeGui(std::shared_ptr<ModelManager> modelManager);
    ~ModelTreeGui();

    // 实现IGUIComponent接口
    void render() override;
    bool isVisible() const override { return myIsVisible; }
    void setVisible(bool visible) override { myIsVisible = visible; }
    void onModelEvent(const ModelEventData& eventData) override;

private:
    void renderObjectNode(const Handle(AIS_InteractiveObject)& theObject, size_t theIndex);
    void renderObjectProperties(const Handle(AIS_InteractiveObject)& theObject);
    void renderObjectVisibility(const Handle(AIS_InteractiveObject)& theObject);
    void renderDisplayMode(const Handle(AIS_InteractiveObject)& theObject);

    std::shared_ptr<ModelManager> myModelManager;
    bool myIsVisible;
    std::map<Handle(AIS_InteractiveObject), Standard_Integer> myDisplayModes;
    std::string myComponentId;
}; 