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
#include <animation/interpolators.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <core/pathmeasure.h>
#include <drawable/pathparser.h>

namespace cdroid{
BaseInterpolator::BaseInterpolator(){
    mChangingConfiguration =0;
}

void BaseInterpolator::setChangingConfiguration(int changingConfiguration){
    mChangingConfiguration = changingConfiguration;
}

int BaseInterpolator::getChangingConfiguration(){
    return mChangingConfiguration;
}

AccelerateInterpolator::AccelerateInterpolator(Context*ctx,const AttributeSet&atts){
    mFactor = atts.getFloat("factor",1.f);
    mDoubleFactor = mFactor*2.f;   
}

AccelerateInterpolator::AccelerateInterpolator(double f){
    mFactor = float(f);
    mDoubleFactor = float(f*2.0);
}

float AccelerateInterpolator::getInterpolation(float input)const{
    if (mFactor == 1.0f) {
        return input * input;
    } else {
        return (float)std::pow(input, mDoubleFactor);
    }
}

DecelerateInterpolator::DecelerateInterpolator(Context*ctx,const AttributeSet&atts){
    mFactor = atts.getFloat("factor",1.f);
}

DecelerateInterpolator::DecelerateInterpolator(float factor) {
    mFactor = factor;
}

float DecelerateInterpolator::getInterpolation(float input)const{
    float result;
    if (mFactor == 1.0f) {
        result = (float)(1.0f - (1.0f - input) * (1.0f - input));
    } else {
        result = (float)(1.0f - std::pow((1.0f - input), 2 * mFactor));
    }
    return result;
}

AnticipateInterpolator::AnticipateInterpolator(Context*ctx,const AttributeSet&atts){
    mTension = atts.getFloat("tension",2.f);
}

AnticipateInterpolator::AnticipateInterpolator(float tension){
    mTension = tension;
}

float AnticipateInterpolator::getInterpolation(float t)const{
    return t * t * ((mTension + 1) * t - mTension);
}

CycleInterpolator::CycleInterpolator(Context*ctx,const AttributeSet&atts){
    mCycles = atts.getFloat("cycles",1.f);
}

CycleInterpolator::CycleInterpolator(float cycles) {
    mCycles = cycles;
}

float CycleInterpolator::getInterpolation(float input)const{
    return (float)(sin(2 * mCycles * M_PI * input));
}

OvershootInterpolator::OvershootInterpolator(Context*ctx,const AttributeSet&atts){
    mTension = atts.getFloat("tension",2.f);
}

OvershootInterpolator::OvershootInterpolator(float tension) {
    mTension = tension;
}

float OvershootInterpolator::getInterpolation(float t)const{
    t -= 1.0f;
    return t * t * ((mTension + 1) * t + mTension) + 1.0f;
}

AnticipateOvershootInterpolator::AnticipateOvershootInterpolator(Context*ctx,const AttributeSet&atts){
    mTension = atts.getFloat("tension",2.f)*atts.getFloat("extraTension",1.5f);
}

AnticipateOvershootInterpolator::AnticipateOvershootInterpolator(float tension, float extraTension) {
    mTension = tension * extraTension;
}

float AnticipateOvershootInterpolator::a(float t, float s) {
    return t * t * ((s + 1) * t - s);
}

float AnticipateOvershootInterpolator::o(float t, float s) {
    return t * t * ((s + 1) * t + s);
}

float AnticipateOvershootInterpolator::getInterpolation(float t)const{
    if (t < 0.5f) return 0.5f * a(t * 2.0f, mTension);
    else return 0.5f * (o(t * 2.0f - 2.0f, mTension) + 2.0f);
}


float BounceInterpolator::bounce(float t) {
    return t * t * 8.0f;
}

float BounceInterpolator::getInterpolation(float t)const{
    // _b(t) = t * t * 8
    // bs(t) = _b(t) for t < 0.3535
    // bs(t) = _b(t - 0.54719) + 0.7 for t < 0.7408
    // bs(t) = _b(t - 0.8526) + 0.9 for t < 0.9644
    // bs(t) = _b(t - 1.0435) + 0.95 for t <= 1.0
    // b(t) = bs(t * 1.1226)
    t *= 1.1226f;
    if (t < 0.3535f) return bounce(t);
    else if (t < 0.7408f) return bounce(t - 0.54719f) + 0.7f;
    else if (t < 0.9644f) return bounce(t - 0.8526f) + 0.9f;
    else return bounce(t - 1.0435f) + 0.95f;
}

float AccelerateDecelerateInterpolator::getInterpolation(float input)const{
    return (float)(cos((input + 1) * M_PI) / 2.0f) + 0.5f;
}

PathInterpolator::PathInterpolator(cdroid::Path&path){
}

PathInterpolator::PathInterpolator(float controlX, float controlY) {
    initQuad(controlX, controlY);
}

PathInterpolator::PathInterpolator(float controlX1, float controlY1, float controlX2, float controlY2) {
    initCubic(controlX1, controlY1, controlX2, controlY2);
}

PathInterpolator::PathInterpolator(Context*ctx,const AttributeSet&a){
    if(a.hasAttribute("pathData")){
        std::string pathData = a.getString("pathData");
        auto path = PathParser::createPathFromPathData(pathData);
        if (path == nullptr) {
            throw std::runtime_error("The path is null, which is created from " + pathData);
        }
        initPath(*path);
    }else{
        if (!a.hasAttribute("controlX1")) {
            throw "pathInterpolator requires the controlX1 attribute";
        } else if (!a.hasAttribute("controlY1")) {
            throw "pathInterpolator requires the controlY1 attribute";
        }
        const float x1 = a.getFloat("controlX1", 0);
        const float y1 = a.getFloat("controlY1", 0);

        const bool hasX2 = a.hasAttribute("controlX2");
        const bool hasY2 = a.hasAttribute("controlY2");

        if (hasX2 != hasY2) {
            throw "pathInterpolator requires both controlX2 and controlY2 for cubic Beziers.";
        }

        if (!hasX2) {
            initQuad(x1, y1);
        } else {
            float x2 = a.getFloat("controlX2", 0);
            float y2 = a.getFloat("controlY2", 0);
            initCubic(x1, y1, x2, y2);
        }
    }
}

void PathInterpolator::initQuad(float controlX, float controlY) {
    Path path;
    path.move_to(0, 0);
    path.quad_to(controlX, controlY, 1.f, 1.f);
    initPath(path);
}

void PathInterpolator::initCubic(float x1, float y1, float x2, float y2) {
    Path path;
    path.move_to(0, 0);
    path.curve_to(x1, y1, x2, y2, 1.f, 1.f);
    initPath(path);
}

void PathInterpolator::initPath(cdroid::Path&path){
    std::vector<float> pointComponents;
    path.approximate(pointComponents,PRECISION);

    const int numPoints = pointComponents.size() / 3;
    if (pointComponents[1] != 0 || pointComponents[2] != 0
            || pointComponents[pointComponents.size() - 2] != 1
            || pointComponents[pointComponents.size() - 1] != 1) {
        throw std::invalid_argument("The Path must start at (0,0) and end at (1,1)");
    }

    mX.resize(numPoints);
    mY.resize(numPoints);
    float prevX = 0;
    float prevFraction = 0;
    int componentIndex = 0;
    for (int i = 0; i < numPoints; i++) {
        float fraction = pointComponents[componentIndex++];
        float x = pointComponents[componentIndex++];
        float y = pointComponents[componentIndex++];
        if (fraction == prevFraction && x != prevX) {
            throw std::invalid_argument("The Path cannot have discontinuity in the X axis.");
        }
        if (x < prevX) {
            throw std::invalid_argument("The Path cannot loop back on itself.");
        }
        mX[i] = x;
        mY[i] = y;
        prevX = x;
        prevFraction = fraction;
    }
}

float PathInterpolator::getInterpolation(float t)const {
    if (t <= 0) {
        return 0;
    } else if (t >= 1) {
        return 1;
    }
    // Do a binary search for the correct x to interpolate between.
    int startIndex = 0;
    int endIndex = mX.size() - 1;

    while (endIndex - startIndex > 1) {
        int midIndex = (startIndex + endIndex) / 2;
        if (t < mX[midIndex]) {
            endIndex = midIndex;
        } else {
            startIndex = midIndex;
        }
    }

    float xRange = mX[endIndex] - mX[startIndex];
    if (xRange == 0) {
        return mY[startIndex];
    }

    float tInRange = t - mX[startIndex];
    float fraction = tInRange / xRange;
    float startY = mY[startIndex];
    float endY = mY[endIndex];
    return startY + (fraction * (endY - startY));
}

BackGestureInterpolator::BackGestureInterpolator()
    :PathInterpolator(0.1f,0.1f,0.f,1.f){
}
//////////////////////////////////////////////////////////////////////////////////////////////

LookupTableInterpolator::LookupTableInterpolator(const std::vector<float>& values) {
    mValues = values;
    mStepSize = 1.f / (mValues.size() - 1);
}

LookupTableInterpolator::LookupTableInterpolator(const float*values,int count){
    mValues.assign(values,values+count);
    mStepSize = 1.f / (count - 1);
}

float LookupTableInterpolator::getInterpolation(float input)const{
    if (input >= 1.0f) {
        return 1.0f;
    }
    if (input <= .0f) {
        return .0f;
    }

    // Calculate index - We use min with length - 2 to avoid IndexOutOfBoundsException when
    // we lerp (linearly interpolate) in the return statement
    int position = std::min((unsigned int) (input * (mValues.size() - 1)), (unsigned int)(mValues.size() - 2));

    // Calculate values to account for small offsets as the lookup table has discrete values
    float quantized = position * mStepSize;
    float diff = input - quantized;
    float weight = diff / mStepSize;

    // Linearly interpolate between the table values
    return mValues[position] + weight * (mValues[position + 1] - mValues[position]);
}

static float VALUES1[] ={
    0.0000f, 0.0001f, 0.0002f, 0.0005f, 0.0009f, 0.0014f, 0.0020f,
    0.0027f, 0.0036f, 0.0046f, 0.0058f, 0.0071f, 0.0085f, 0.0101f,
    0.0118f, 0.0137f, 0.0158f, 0.0180f, 0.0205f, 0.0231f, 0.0259f,
    0.0289f, 0.0321f, 0.0355f, 0.0391f, 0.0430f, 0.0471f, 0.0514f,
    0.0560f, 0.0608f, 0.0660f, 0.0714f, 0.0771f, 0.0830f, 0.0893f,
    0.0959f, 0.1029f, 0.1101f, 0.1177f, 0.1257f, 0.1339f, 0.1426f,
    0.1516f, 0.1610f, 0.1707f, 0.1808f, 0.1913f, 0.2021f, 0.2133f,
    0.2248f, 0.2366f, 0.2487f, 0.2611f, 0.2738f, 0.2867f, 0.2998f,
    0.3131f, 0.3265f, 0.3400f, 0.3536f, 0.3673f, 0.3810f, 0.3946f,
    0.4082f, 0.4217f, 0.4352f, 0.4485f, 0.4616f, 0.4746f, 0.4874f,
    0.5000f, 0.5124f, 0.5246f, 0.5365f, 0.5482f, 0.5597f, 0.5710f,
    0.5820f, 0.5928f, 0.6033f, 0.6136f, 0.6237f, 0.6335f, 0.6431f,
    0.6525f, 0.6616f, 0.6706f, 0.6793f, 0.6878f, 0.6961f, 0.7043f,
    0.7122f, 0.7199f, 0.7275f, 0.7349f, 0.7421f, 0.7491f, 0.7559f,
    0.7626f, 0.7692f, 0.7756f, 0.7818f, 0.7879f, 0.7938f, 0.7996f,
    0.8053f, 0.8108f, 0.8162f, 0.8215f, 0.8266f, 0.8317f, 0.8366f,
    0.8414f, 0.8461f, 0.8507f, 0.8551f, 0.8595f, 0.8638f, 0.8679f,
    0.8720f, 0.8760f, 0.8798f, 0.8836f, 0.8873f, 0.8909f, 0.8945f,
    0.8979f, 0.9013f, 0.9046f, 0.9078f, 0.9109f, 0.9139f, 0.9169f,
    0.9198f, 0.9227f, 0.9254f, 0.9281f, 0.9307f, 0.9333f, 0.9358f,
    0.9382f, 0.9406f, 0.9429f, 0.9452f, 0.9474f, 0.9495f, 0.9516f,
    0.9536f, 0.9556f, 0.9575f, 0.9594f, 0.9612f, 0.9629f, 0.9646f,
    0.9663f, 0.9679f, 0.9695f, 0.9710f, 0.9725f, 0.9739f, 0.9753f,
    0.9766f, 0.9779f, 0.9791f, 0.9803f, 0.9815f, 0.9826f, 0.9837f,
    0.9848f, 0.9858f, 0.9867f, 0.9877f, 0.9885f, 0.9894f, 0.9902f,
    0.9910f, 0.9917f, 0.9924f, 0.9931f, 0.9937f, 0.9944f, 0.9949f,
    0.9955f, 0.9960f, 0.9964f, 0.9969f, 0.9973f, 0.9977f, 0.9980f,
    0.9984f, 0.9986f, 0.9989f, 0.9991f, 0.9993f, 0.9995f, 0.9997f,
    0.9998f, 0.9999f, 0.9999f, 1.0000f, 1.0000f
};

static float VALUES2[] ={
    0.0000f, 0.0222f, 0.0424f, 0.0613f, 0.0793f, 0.0966f, 0.1132f,
    0.1293f, 0.1449f, 0.1600f, 0.1747f, 0.1890f, 0.2029f, 0.2165f,
    0.2298f, 0.2428f, 0.2555f, 0.2680f, 0.2802f, 0.2921f, 0.3038f,
    0.3153f, 0.3266f, 0.3377f, 0.3486f, 0.3592f, 0.3697f, 0.3801f,
    0.3902f, 0.4002f, 0.4100f, 0.4196f, 0.4291f, 0.4385f, 0.4477f,
    0.4567f, 0.4656f, 0.4744f, 0.4831f, 0.4916f, 0.5000f, 0.5083f,
    0.5164f, 0.5245f, 0.5324f, 0.5402f, 0.5479f, 0.5555f, 0.5629f,
    0.5703f, 0.5776f, 0.5847f, 0.5918f, 0.5988f, 0.6057f, 0.6124f,
    0.6191f, 0.6257f, 0.6322f, 0.6387f, 0.6450f, 0.6512f, 0.6574f,
    0.6635f, 0.6695f, 0.6754f, 0.6812f, 0.6870f, 0.6927f, 0.6983f,
    0.7038f, 0.7093f, 0.7147f, 0.7200f, 0.7252f, 0.7304f, 0.7355f,
    0.7406f, 0.7455f, 0.7504f, 0.7553f, 0.7600f, 0.7647f, 0.7694f,
    0.7740f, 0.7785f, 0.7829f, 0.7873f, 0.7917f, 0.7959f, 0.8002f,
    0.8043f, 0.8084f, 0.8125f, 0.8165f, 0.8204f, 0.8243f, 0.8281f,
    0.8319f, 0.8356f, 0.8392f, 0.8429f, 0.8464f, 0.8499f, 0.8534f,
    0.8568f, 0.8601f, 0.8634f, 0.8667f, 0.8699f, 0.8731f, 0.8762f,
    0.8792f, 0.8823f, 0.8852f, 0.8882f, 0.8910f, 0.8939f, 0.8967f,
    0.8994f, 0.9021f, 0.9048f, 0.9074f, 0.9100f, 0.9125f, 0.9150f,
    0.9174f, 0.9198f, 0.9222f, 0.9245f, 0.9268f, 0.9290f, 0.9312f,
    0.9334f, 0.9355f, 0.9376f, 0.9396f, 0.9416f, 0.9436f, 0.9455f,
    0.9474f, 0.9492f, 0.9510f, 0.9528f, 0.9545f, 0.9562f, 0.9579f,
    0.9595f, 0.9611f, 0.9627f, 0.9642f, 0.9657f, 0.9672f, 0.9686f,
    0.9700f, 0.9713f, 0.9726f, 0.9739f, 0.9752f, 0.9764f, 0.9776f,
    0.9787f, 0.9798f, 0.9809f, 0.9820f, 0.9830f, 0.9840f, 0.9849f,
    0.9859f, 0.9868f, 0.9876f, 0.9885f, 0.9893f, 0.9900f, 0.9908f,
    0.9915f, 0.9922f, 0.9928f, 0.9934f, 0.9940f, 0.9946f, 0.9951f,
    0.9956f, 0.9961f, 0.9966f, 0.9970f, 0.9974f, 0.9977f, 0.9981f,
    0.9984f, 0.9987f, 0.9989f, 0.9992f, 0.9994f, 0.9995f, 0.9997f,
    0.9998f, 0.9999f, 0.9999f, 1.0000f, 1.0000f
};

static float VALUES3[] ={
    0.0000f, 0.0001f, 0.0002f, 0.0005f, 0.0008f, 0.0013f, 0.0018f,
    0.0024f, 0.0032f, 0.0040f, 0.0049f, 0.0059f, 0.0069f, 0.0081f,
    0.0093f, 0.0106f, 0.0120f, 0.0135f, 0.0151f, 0.0167f, 0.0184f,
    0.0201f, 0.0220f, 0.0239f, 0.0259f, 0.0279f, 0.0300f, 0.0322f,
    0.0345f, 0.0368f, 0.0391f, 0.0416f, 0.0441f, 0.0466f, 0.0492f,
    0.0519f, 0.0547f, 0.0574f, 0.0603f, 0.0632f, 0.0662f, 0.0692f,
    0.0722f, 0.0754f, 0.0785f, 0.0817f, 0.0850f, 0.0884f, 0.0917f,
    0.0952f, 0.0986f, 0.1021f, 0.1057f, 0.1093f, 0.1130f, 0.1167f,
    0.1205f, 0.1243f, 0.1281f, 0.1320f, 0.1359f, 0.1399f, 0.1439f,
    0.1480f, 0.1521f, 0.1562f, 0.1604f, 0.1647f, 0.1689f, 0.1732f,
    0.1776f, 0.1820f, 0.1864f, 0.1909f, 0.1954f, 0.1999f, 0.2045f,
    0.2091f, 0.2138f, 0.2184f, 0.2232f, 0.2279f, 0.2327f, 0.2376f,
    0.2424f, 0.2473f, 0.2523f, 0.2572f, 0.2622f, 0.2673f, 0.2723f,
    0.2774f, 0.2826f, 0.2877f, 0.2929f, 0.2982f, 0.3034f, 0.3087f,
    0.3141f, 0.3194f, 0.3248f, 0.3302f, 0.3357f, 0.3412f, 0.3467f,
    0.3522f, 0.3578f, 0.3634f, 0.3690f, 0.3747f, 0.3804f, 0.3861f,
    0.3918f, 0.3976f, 0.4034f, 0.4092f, 0.4151f, 0.4210f, 0.4269f,
    0.4329f, 0.4388f, 0.4448f, 0.4508f, 0.4569f, 0.4630f, 0.4691f,
    0.4752f, 0.4814f, 0.4876f, 0.4938f, 0.5000f, 0.5063f, 0.5126f,
    0.5189f, 0.5252f, 0.5316f, 0.5380f, 0.5444f, 0.5508f, 0.5573f,
    0.5638f, 0.5703f, 0.5768f, 0.5834f, 0.5900f, 0.5966f, 0.6033f,
    0.6099f, 0.6166f, 0.6233f, 0.6301f, 0.6369f, 0.6436f, 0.6505f,
    0.6573f, 0.6642f, 0.6710f, 0.6780f, 0.6849f, 0.6919f, 0.6988f,
    0.7059f, 0.7129f, 0.7199f, 0.7270f, 0.7341f, 0.7413f, 0.7484f,
    0.7556f, 0.7628f, 0.7700f, 0.7773f, 0.7846f, 0.7919f, 0.7992f,
    0.8066f, 0.8140f, 0.8214f, 0.8288f, 0.8363f, 0.8437f, 0.8513f,
    0.8588f, 0.8664f, 0.8740f, 0.8816f, 0.8892f, 0.8969f, 0.9046f,
    0.9124f, 0.9201f, 0.9280f, 0.9358f, 0.9437f, 0.9516f, 0.9595f,
    0.9675f, 0.9755f, 0.9836f, 0.9918f, 1.0000f
};

FastOutSlowInInterpolator::FastOutSlowInInterpolator()
 :LookupTableInterpolator(VALUES1,sizeof(VALUES1)/sizeof(float)){
    
}

LinearOutSlowInInterpolator::LinearOutSlowInInterpolator()
 :LookupTableInterpolator(VALUES2,sizeof(VALUES2)/sizeof(float)){
}

FastOutLinearInInterpolator::FastOutLinearInInterpolator()
 :LookupTableInterpolator(VALUES3,sizeof(VALUES3)/sizeof(float)){
}


//public static final BezierSCurveInterpolator INSTANCE = new BezierSCurveInterpolator();
static const float BEZIERSCURVE_VALUES[] ={
        0.0f, 0.0002f, 0.0009f, 0.0019f, 0.0036f, 0.0059f, 0.0086f, 0.0119f, 0.0157f, 0.0209f,
        0.0257f, 0.0321f, 0.0392f, 0.0469f, 0.0566f, 0.0656f, 0.0768f, 0.0887f, 0.1033f,
        0.1186f, 0.1349f, 0.1519f, 0.1696f, 0.1928f, 0.2121f, 0.237f, 0.2627f, 0.2892f, 0.3109f,
        0.3386f, 0.3667f, 0.3952f, 0.4241f, 0.4474f, 0.4766f, 0.5f, 0.5234f, 0.5468f, 0.5701f,
        0.5933f, 0.6134f, 0.6333f, 0.6531f, 0.6698f, 0.6891f, 0.7054f, 0.7214f, 0.7346f,
        0.7502f, 0.763f, 0.7756f, 0.7879f, 0.8f, 0.8107f, 0.8212f, 0.8326f, 0.8415f, 0.8503f,
        0.8588f, 0.8672f, 0.8754f, 0.8833f, 0.8911f, 0.8977f, 0.9041f, 0.9113f, 0.9165f,
        0.9232f, 0.9281f, 0.9328f, 0.9382f, 0.9434f, 0.9476f, 0.9518f, 0.9557f, 0.9596f,
        0.9632f, 0.9662f, 0.9695f, 0.9722f, 0.9753f, 0.9777f, 0.9805f, 0.9826f, 0.9847f,
        0.9866f, 0.9884f, 0.9901f, 0.9917f, 0.9931f, 0.9944f, 0.9955f, 0.9964f, 0.9973f,
        0.9981f, 0.9986f, 0.9992f, 0.9995f, 0.9998f, 1.0f, 1.0f
};
#define BEZIERSCURVE_LENGH (sizeof(BEZIERSCURVE_VALUES)/sizeof(BEZIERSCURVE_VALUES[0]))
#define BEZIERSCURVE_STEP_SIZE (1.0f / float(BEZIERSCURVE_LENGH - 1))

BezierSCurveInterpolator::BezierSCurveInterpolator() {
}

float BezierSCurveInterpolator::getInterpolation(float input)const{
    if (input >= 1.0f) {
        return 1.0f;
    }

    if (input <= 0.f) {
        return 0.f;
    }

    int position = std::min( int(input * (BEZIERSCURVE_LENGH - 1)), int(BEZIERSCURVE_LENGH - 2));

    float quantized = position * BEZIERSCURVE_STEP_SIZE;
    float difference = input - quantized;
    float weight = difference / BEZIERSCURVE_STEP_SIZE;

    return BEZIERSCURVE_VALUES[position] + weight * (BEZIERSCURVE_VALUES[position + 1] - BEZIERSCURVE_VALUES[position]);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace{
    AccelerateInterpolator mAccelerateInterpolator(1.f);
    AccelerateDecelerateInterpolator mAccelerateDecelerateInterpolator;
    BezierSCurveInterpolator mBezierSCurveInterpolator;
    BounceInterpolator mBounceInterpolator;
    AnticipateInterpolator mAnticipateInterpolator;
    DecelerateInterpolator mDecelerateInterpolator;
    FastOutSlowInInterpolator mFastOutSlowInInterpolator;
    FastOutLinearInInterpolator mFastOutLinearInInterpolator;
    LinearInterpolator mLinearInterpolator;
    LinearOutSlowInInterpolator mLinearOutSlowInInterpolator;
    OvershootInterpolator mOvershootInterpolator;
}

const AnticipateInterpolator*const AnticipateInterpolator::Instance= &mAnticipateInterpolator;
const AccelerateInterpolator*const AccelerateInterpolator::Instance = &mAccelerateInterpolator;
const AccelerateDecelerateInterpolator*const AccelerateDecelerateInterpolator::Instance = &mAccelerateDecelerateInterpolator;
const BounceInterpolator*const BounceInterpolator::Instance = &mBounceInterpolator;
const BezierSCurveInterpolator*const BezierSCurveInterpolator::Instance = &mBezierSCurveInterpolator;
const DecelerateInterpolator*const DecelerateInterpolator::Instance= &mDecelerateInterpolator;
const FastOutSlowInInterpolator*const FastOutSlowInInterpolator::Instance = &mFastOutSlowInInterpolator;
const FastOutLinearInInterpolator*const FastOutLinearInInterpolator::Instance = &mFastOutLinearInInterpolator;
const LinearInterpolator*const LinearInterpolator::Instance = &mLinearInterpolator;
const LinearOutSlowInInterpolator*const LinearOutSlowInInterpolator::Instance = &mLinearOutSlowInInterpolator;
const OvershootInterpolator*const OvershootInterpolator::Instance = &mOvershootInterpolator;

}//endof namespace
