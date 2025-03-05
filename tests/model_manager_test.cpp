#define BOOST_TEST_MODULE ModelManager Tests
#include <boost/test/unit_test.hpp>

#include "model/ModelManager.h"
#include "model/IModel.h"

// Create a simple mock model for testing
class MockModel : public IModel {
public:
    std::vector<std::string> getAllEntityIds() const override {
        return {"entity1", "entity2"};
    }
    
    void removeEntity(const std::string& id) override {
        // Just a mock implementation
    }
};

BOOST_AUTO_TEST_SUITE(model_manager_tests)

BOOST_AUTO_TEST_CASE(create_and_get_model)
{
    // Arrange
    ModelManager manager;
    const std::string modelId = "test_model";
    
    // Act
    auto model = manager.createModel<MockModel>(modelId);
    auto retrievedModel = manager.getModel(modelId);
    
    // Assert
    BOOST_CHECK(model != nullptr);
    BOOST_CHECK(retrievedModel != nullptr);
    BOOST_CHECK_EQUAL(model.get(), retrievedModel.get());
}

BOOST_AUTO_TEST_CASE(remove_model)
{
    // Arrange
    ModelManager manager;
    const std::string modelId = "test_model";
    manager.createModel<MockModel>(modelId);
    
    // Act
    manager.removeModel(modelId);
    auto retrievedModel = manager.getModel(modelId);
    
    // Assert
    BOOST_CHECK(retrievedModel == nullptr);
}

BOOST_AUTO_TEST_CASE(get_all_model_ids)
{
    // Arrange
    ModelManager manager;
    manager.createModel<MockModel>("model1");
    manager.createModel<MockModel>("model2");
    manager.createModel<MockModel>("model3");
    
    // Act
    auto modelIds = manager.getAllModelIds();
    
    // Assert
    BOOST_CHECK_EQUAL(modelIds.size(), 3);
    BOOST_CHECK(std::find(modelIds.begin(), modelIds.end(), "model1") != modelIds.end());
    BOOST_CHECK(std::find(modelIds.begin(), modelIds.end(), "model2") != modelIds.end());
    BOOST_CHECK(std::find(modelIds.begin(), modelIds.end(), "model3") != modelIds.end());
}

BOOST_AUTO_TEST_SUITE_END() 