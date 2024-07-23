#include <map>
#include <cfloat>
#include <gesture/prediction.h>
#include <gesture/instance.h>
#include <gesture/instancelearner.h>
#include <gesture/gesturestore.h>
#include <gesture/gestureutils.h>

namespace cdroid{
    /*private static final Comparator<Prediction> sComparator = new Comparator<Prediction>() {
        public int compare(Prediction object1, Prediction object2) {
            double score1 = object1.score;
            double score2 = object2.score;
            if (score1 > score2) {
                return -1;
            } else if (score1 < score2) {
                return 1;
            } else {
                return 0;
            }
        }
    }*/

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
        predictions.push_back(/*Prediction*/{it.first, it.second});
    }

    //Collections.sort(predictions, sComparator);
    return predictions;
}

}/*endof namespace*/
