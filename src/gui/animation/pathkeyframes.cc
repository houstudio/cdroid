/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <animation/pathkeyframes.h>
#include <stdexcept>

namespace cdroid{

std::vector<Keyframe*>& PathKeyframes::emptyKeyframes(){
    static std::vector<Keyframe*> empty;
    return empty;
}

PathKeyframes::PathKeyframes(const Cairo::RefPtr<Path>&path,float error){
    if (path) {
        path->approximate(mKeyframeData, error);
    }
    if (mKeyframeData.empty()) {
        // AOSP: "The path must not be null or empty" — CDROID's Path has no
        // isEmpty(); an empty approximation means the same thing.
        throw std::invalid_argument("The path must not be null or empty");
    }
}

std::vector<Keyframe*>& PathKeyframes::getKeyframes(){
    // AOSP: keyframe data lives in mKeyframeData, not in Keyframe objects.
    return emptyKeyframes();
}

const PointF&PathKeyframes::pointForFraction(float fraction){
    const int numPoints = (int)(mKeyframeData.size() / NUM_COMPONENTS);
    if (fraction < 0) {
        return interpolateInRange(fraction, 0, 1);
    } else if (fraction > 1) {
        return interpolateInRange(fraction, numPoints - 2, numPoints - 1);
    } else if (fraction == 0) {
        return pointForIndex(0);
    } else if (fraction == 1) {
        return pointForIndex(numPoints - 1);
    }
    // Binary search for the correct section
    int low = 0;
    int high = numPoints - 1;
    while (low <= high) {
        const int mid = (low + high) / 2;
        const float midFraction = mKeyframeData[mid * NUM_COMPONENTS + FRACTION_OFFSET];
        if (fraction < midFraction) {
            high = mid - 1;
        } else if (fraction > midFraction) {
            low = mid + 1;
        } else {
            return pointForIndex(mid);
        }
    }
    // now high is below the fraction and low is above the fraction
    return interpolateInRange(fraction, high, low);
}

const AnimateValue&PathKeyframes::getValue(float fraction){
    mAnimatedValue = pointForFraction(fraction);
    return mAnimatedValue;
}

void PathKeyframes::setEvaluator(TypeEvaluatorFn){
    // AOSP: no-op — the sampling data fully defines the values.
}

int PathKeyframes::getType()const{
    // AOSP: PointF.class — no Property::*_TYPE constant exists for it.
    return Property::UNDEFINED;
}

Keyframes*PathKeyframes::clone()const{
    // AOSP super.clone(): shallow; the sample data is immutable either way.
    return new PathKeyframes(*this);
}

const PointF&PathKeyframes::interpolateInRange(float fraction,int startIndex,int endIndex){
    const float startFraction = mKeyframeData[startIndex * NUM_COMPONENTS + FRACTION_OFFSET];
    const float endFraction   = mKeyframeData[endIndex * NUM_COMPONENTS + FRACTION_OFFSET];
    const float intervalFraction = (fraction - startFraction)/(endFraction - startFraction);
    const float startX = mKeyframeData[startIndex * NUM_COMPONENTS + X_OFFSET];
    const float endX   = mKeyframeData[endIndex * NUM_COMPONENTS + X_OFFSET];
    const float startY = mKeyframeData[startIndex * NUM_COMPONENTS + Y_OFFSET];
    const float endY   = mKeyframeData[endIndex * NUM_COMPONENTS + Y_OFFSET];
    const float x = interpolate(intervalFraction, startX, endX);
    const float y = interpolate(intervalFraction, startY, endY);
    mTempPointF.x = x;
    mTempPointF.y = y;
    return mTempPointF;
}

const PointF&PathKeyframes::pointForIndex(int index){
    const int base = index * NUM_COMPONENTS;
    mTempPointF.x = mKeyframeData[base + X_OFFSET];
    mTempPointF.y = mKeyframeData[base + Y_OFFSET];
    return mTempPointF;
}

float PathKeyframes::interpolate(float fraction,float startValue,float endValue){
    const float diff = endValue - startValue;
    return startValue + diff * fraction;
}

FloatKeyframes*PathKeyframes::createXFloatKeyframes(){
    return new XFloatKeyframes(shared_from_this());
}

FloatKeyframes*PathKeyframes::createYFloatKeyframes(){
    return new YFloatKeyframes(shared_from_this());
}

IntKeyframes*PathKeyframes::createXIntKeyframes(){
    return new XIntKeyframes(shared_from_this());
}

IntKeyframes*PathKeyframes::createYIntKeyframes(){
    return new YIntKeyframes(shared_from_this());
}

PathKeyframes::FloatKeyframesBase::FloatKeyframesBase(const std::shared_ptr<PathKeyframes>&path)
    :mPath(path){
}

void PathKeyframes::FloatKeyframesBase::setEvaluator(TypeEvaluatorFn){
}

std::vector<Keyframe*>& PathKeyframes::FloatKeyframesBase::getKeyframes(){
    return emptyKeyframes();
}

int PathKeyframes::FloatKeyframesBase::getType()const{
    return Property::FLOAT_TYPE;
}

const AnimateValue&PathKeyframes::FloatKeyframesBase::getValue(float fraction){
    mValue = getFloatValue(fraction);
    return mValue;
}

PathKeyframes::IntKeyframesBase::IntKeyframesBase(const std::shared_ptr<PathKeyframes>&path)
    :mPath(path){
}

void PathKeyframes::IntKeyframesBase::setEvaluator(TypeEvaluatorFn){
}

std::vector<Keyframe*>& PathKeyframes::IntKeyframesBase::getKeyframes(){
    return emptyKeyframes();
}

int PathKeyframes::IntKeyframesBase::getType()const{
    return Property::INT_TYPE;
}

const AnimateValue&PathKeyframes::IntKeyframesBase::getValue(float fraction){
    mValue = getIntValue(fraction);
    return mValue;
}

PathKeyframes::XFloatKeyframes::XFloatKeyframes(const std::shared_ptr<PathKeyframes>&path)
    :FloatKeyframesBase(path){
}

float PathKeyframes::XFloatKeyframes::getFloatValue(float fraction){
    return mPath->pointForFraction(fraction).x;
}

Keyframes*PathKeyframes::XFloatKeyframes::clone()const{
    return new XFloatKeyframes(mPath);
}

PathKeyframes::YFloatKeyframes::YFloatKeyframes(const std::shared_ptr<PathKeyframes>&path)
    :FloatKeyframesBase(path){
}

float PathKeyframes::YFloatKeyframes::getFloatValue(float fraction){
    return mPath->pointForFraction(fraction).y;
}

Keyframes*PathKeyframes::YFloatKeyframes::clone()const{
    return new YFloatKeyframes(mPath);
}

PathKeyframes::XIntKeyframes::XIntKeyframes(const std::shared_ptr<PathKeyframes>&path)
    :IntKeyframesBase(path){
}

int PathKeyframes::XIntKeyframes::getIntValue(float fraction){
    return (int)std::round(mPath->pointForFraction(fraction).x);
}

Keyframes*PathKeyframes::XIntKeyframes::clone()const{
    return new XIntKeyframes(mPath);
}

PathKeyframes::YIntKeyframes::YIntKeyframes(const std::shared_ptr<PathKeyframes>&path)
    :IntKeyframesBase(path){
}

int PathKeyframes::YIntKeyframes::getIntValue(float fraction){
    return (int)std::round(mPath->pointForFraction(fraction).y);
}

Keyframes*PathKeyframes::YIntKeyframes::clone()const{
    return new YIntKeyframes(mPath);
}

}//endof namespace
