#ifndef __GESTURE_INSTANCE_H__
#define __GESTURE_INSTANCE_H__
#include <string>
#include <vector>
#include <gesture/learner.h>
#include <gesture/instancelearner.h>
#include <gesture/gesturestore.h>
namespace cdroid{
class Instance {
private:
    static constexpr int SEQUENCE_SAMPLE_SIZE = 16;
    static constexpr int PATCH_SAMPLE_SIZE = 16;
private:
    Instance(long id,const std::vector<float>& sample,const std::string& sampleName);
    void normalize();
    static std::vector<float> spatialSampler(const Gesture& gesture);
    static std::vector<float> temporalSampler(int orientationType,const Gesture& gesture);
protected:
    // the feature vector
    std::vector<float> vector;
    // the label can be null
    const std::string label;
    // the id of the instance
    long id;
    friend class Learner;
    friend class GestureStore;
    friend class InstanceLearner;
public:
    /**
     * create a learning instance for a single stroke gesture
     *
     * @param gesture
     * @param label
     * @return the instance
     */
    static Instance* createInstance(int sequenceType, int orientationType,const Gesture& gesture,const std::string& label);
};
}/*endof namespace*/
#endif /*__GESTURE_INSTANCE_H__*/
