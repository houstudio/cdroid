#include <gesture/learner.h>
#include <gesture/instance.h>
namespace cdroid{
void Learner::addInstance(Instance* instance) {
    mInstances.push_back(instance);
}

/**
 * Retrieve all the instances
 *
 * @return instances
 */
std::vector<Instance*>& Learner::getInstances() {
    return mInstances;
}

/**
 * Remove an instance based on its id
 *
 * @param id
 */
void Learner::removeInstance(long id) {
    for (auto it = mInstances.begin(); it!=mInstances.end(); it++) {
        if ((*it)->id == id) {
            mInstances.erase(it);
            return;
        }
    }
}

/**
 * Remove all the instances of a category
 *
 * @param name the category name
 */
void Learner::removeInstances(const std::string& name) {
    for (auto it = mInstances.begin(); it !=mInstances.end();) {
        // the label can be null, as specified in Instance
        if ((*it)->label.compare(name)==0) {
            it = mInstances.erase(it);
        }else it++;
    }
}

}
