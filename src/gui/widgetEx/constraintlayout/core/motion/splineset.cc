#include <widgetEx/constraintlayout/core/motion/splineset.h>
#include <algorithm>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>
namespace cdroid {
void SplineSet::setPoint(int position, float value) {
    mTimePoints.push_back(position);
    mValues.push_back(value);
}
void SplineSet::setup(int curveType) {
    if (mTimePoints.empty()) return;
    // sort by time (keeping value paired).
    std::vector<std::pair<int,float>> pairs;
    for (size_t i = 0; i < mTimePoints.size(); i++) pairs.emplace_back(mTimePoints[i], mValues[i]);
    std::sort(pairs.begin(), pairs.end());
    // dedup by time.
    std::vector<double> time;
    std::vector<std::vector<double>> y;
    for (size_t i = 0; i < pairs.size(); i++) {
        if (i > 0 && pairs[i].first == pairs[i-1].first) continue;
        time.push_back(pairs[i].first * 1e-2);
        y.push_back({(double)pairs[i].second});
    }
    mCurveFit = CurveFit::get(curveType, time, y);
}
float SplineSet::get(float t) const {
    if (mCurveFit == nullptr) return 0;
    return (float) mCurveFit->getPos(t, 0);
}
float SplineSet::getSlope(float t) const {
    if (mCurveFit == nullptr) return 0;
    return (float) mCurveFit->getSlope(t, 0);
}
void SplineSet::setProperty(TypedValues* widget, float t) {
    if (widget == nullptr) return;
    int id = widget->getId(mType);
    widget->setValue(id, get(t));
}
} // namespace cdroid
