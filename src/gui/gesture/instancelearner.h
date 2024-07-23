#ifndef __INSTANCE_LEARNER_H__
#define __INSTANCE_LEARNER_H__
#include <vector>
#include <gesture/learner.h>
namespace cdroid{
class InstanceLearner:public Learner {
public:
    std::vector<Prediction> classify(int sequenceType, int orientationType, const std::vector<float>& vector)override;
};
}/*endof namespace*/
#endif/*__INSTANCE_LEARNER_H__*/
