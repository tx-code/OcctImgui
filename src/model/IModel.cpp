#include "IModel.h"

void IModel::addChangeListener(ChangeListener listener) {
    myChangeListeners.push_back(listener);
}

void IModel::notifyChange(const std::string& entityId) {
    for (auto& listener : myChangeListeners) {
        listener(entityId);
    }
} 