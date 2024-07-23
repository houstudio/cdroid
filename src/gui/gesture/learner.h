#ifndef __LEARNER_H__
#define __LEARNER_H__
#include <vector>
#include <gesture/prediction.h>
namespace cdroid{
class Instance;

class Learner {
private:
    std::vector<Instance*> mInstances;
public:
    /**
     * Add an instance to the learner
     *
     * @param instance
     */
    void addInstance(Instance* instance);
    /**
     * Retrieve all the instances
     *
     * @return instances
     */
    std::vector<Instance*>& getInstances();
    /**
     * Remove an instance based on its id
     *
     * @param id
     */
    void removeInstance(long id);
    /**
     * Remove all the instances of a category
     *
     * @param name the category name
     */
    void removeInstances(const std::string& name);
    virtual std::vector<Prediction> classify(int sequenceType, int orientationType, const std::vector<float>& vector)=0;
};
}/*endof namespace*/
#endif/*__LEARNER_H__*/
