#include <app/espresso/idlingresource.h>

#include <mutex>

namespace cdroid {
namespace espresso {

IdlingResourceRegistry& IdlingResourceRegistry::getInstance() {
    static IdlingResourceRegistry instance;
    return instance;
}

void IdlingResourceRegistry::registerResources(const std::vector<IdlingResourcePtr>& resources) {
    for (const IdlingResourcePtr& resource : resources) {
        mResources.push_back(resource);
    }
}

bool IdlingResourceRegistry::unregisterResources(const std::vector<IdlingResourcePtr>& resources) {
    bool allRemoved = true;
    for (const IdlingResourcePtr& resource : resources) {
        bool removed = false;
        for (size_t i = 0; i < mResources.size(); i++) {
            if (mResources[i] == resource) {
                mResources.erase(mResources.begin() + i);
                removed = true;
                break;
            }
        }
        allRemoved = allRemoved && removed;
    }
    return allRemoved;
}

std::vector<IdlingResourcePtr> IdlingResourceRegistry::getResources() const {
    return mResources;
}

bool IdlingResourceRegistry::allResourcesAreIdle() const {
    for (const IdlingResourcePtr& resource : mResources) {
        if (!resource->isIdleNow()) return false;
    }
    return true;
}

std::vector<std::string> IdlingResourceRegistry::getBusyResourceNames() const {
    std::vector<std::string> busy;
    for (const IdlingResourcePtr& resource : mResources) {
        if (!resource->isIdleNow()) busy.push_back(resource->getName());
    }
    return busy;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
