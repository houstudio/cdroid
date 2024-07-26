#include <map>
#include <cfloat>
#include <algorithm>
#include <gesture/prediction.h>
#include <gesture/instance.h>
#include <gesture/instancelearner.h>
#include <gesture/gesturestore.h>
#include <gesture/gestureutils.h>

namespace cdroid{

static bool compareScore(const Prediction& a, const Prediction& b) {
    return a.score > b.score; /*In Descending Order*/
}

std::vector<Prediction> InstanceLearner::classify(int sequenceType, int orientationType, const std::vector<float>& vector) {
    std::vector<Prediction> predictions ;
    std::vector<Instance*>& instances = getInstances();
    const int count = instances.size();
    std::map<const std::string, double> label2score;
    for (int i = 0; i < count; i++) {
        Instance* sample = instances.at(i);
        if (sample->vector.size() != vector.size()) {
            continue;
        }
        double distance;
        if (sequenceType == GestureStore::SEQUENCE_SENSITIVE) {
            distance = GestureUtils::minimumCosineDistance(sample->vector, vector, orientationType);
        } else {
            distance = GestureUtils::squaredEuclideanDistance(sample->vector, vector);
        }
        double weight;
        if (distance == 0) {
            weight = FLT_MAX;//Double.MAX_VALUE;
        } else {
            weight = 1.0 / distance;
        }
        auto its = label2score.find(sample->label);
        if ((its==label2score.end()) || (weight > its->second)) {
            label2score.insert({sample->label, weight});
        }
    }

    // double sum = 0;
    for (auto it:label2score) {
        predictions.push_back({it.first, it.second});
    }

    std::sort(predictions.begin(),predictions.end(),compareScore);
    return predictions;
}

}/*endof namespace*/
