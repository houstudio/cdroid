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
#include <stdlib.h>
#include <random>
#include <view/viewgroup.h>
#include <animation/animationutils.h>
#include <animation/gridlayoutanimationcontroller.h>
#include <core/typedarray.h>
#include <androidfw/typedvalue.h>   // TypedValue (Description::parseValue)
#include <widget/framework_styleable.h>

namespace cdroid{
using namespace cdroid::internal;

GridLayoutAnimationController::GridLayoutAnimationController(Context* context,const AttributeSet& attrs)
  :LayoutAnimationController(context,attrs){
    // AOSP GridLayoutAnimationController: columnDelay/rowDelay are float|fraction
    // (Description::parseValue); direction/directionPriority enums are aapt2-compiled.
    auto a = context->obtainStyledAttributes(attrs, R::styleable::GridLayoutAnimation);

    TypedValue v;
    auto parse = [&](int idx)->Animation::Description {
        if (!a->peekValue(idx, &v)) return Animation::Description::parseValue(nullptr, context);
        return Animation::Description::parseValue(&v, context);
    };
    Animation::Description d = parse(R::styleable::GridLayoutAnimation_columnDelay);
    mColumnDelay = d.value;
    d = parse(R::styleable::GridLayoutAnimation_rowDelay);
    mRowDelay = d.value;
    mDirection = a->getInt(R::styleable::GridLayoutAnimation_direction,
            DIRECTION_LEFT_TO_RIGHT | DIRECTION_TOP_TO_BOTTOM);
    mDirectionPriority = a->getInt(R::styleable::GridLayoutAnimation_directionPriority,
            PRIORITY_NONE);
}

GridLayoutAnimationController::GridLayoutAnimationController(Animation* animation)
 :GridLayoutAnimationController(animation,.5f,.5f){
}

GridLayoutAnimationController::GridLayoutAnimationController(Animation* animation, float columnDelay, float rowDelay)
:LayoutAnimationController(animation){
    mColumnDelay = columnDelay;
    mRowDelay = rowDelay;
}

float GridLayoutAnimationController::getColumnDelay(){
    return mColumnDelay;
}

void GridLayoutAnimationController::setColumnDelay(float columnDelay){
    mColumnDelay = columnDelay;
}

float GridLayoutAnimationController::getRowDelay(){
    return mRowDelay;
}

void GridLayoutAnimationController::setRowDelay(float rowDelay){
    mRowDelay = rowDelay;
}

int GridLayoutAnimationController::getDirection(){
    return mDirection;
}

void GridLayoutAnimationController::setDirection(int direction){
    mDirection = direction;
}

int GridLayoutAnimationController::getDirectionPriority(){
    return mDirectionPriority;
}

void GridLayoutAnimationController::setDirectionPriority(int directionPriority){
    mDirectionPriority = directionPriority;
}

bool GridLayoutAnimationController::willOverlap(){
    return mColumnDelay < 1.0f || mRowDelay < 1.0f;
}

int64_t GridLayoutAnimationController::getDelayForView(View* view){
    ViewGroup::LayoutParams* lp = view->getLayoutParams();
    AnimationParameters* params = (AnimationParameters*) lp->layoutAnimationParameters;

    if (params == nullptr)  return 0;
     

    const int column = getTransformedColumnIndex(params);
    const int row = getTransformedRowIndex(params);

    const int rowsCount = params->rowsCount;
    const int columnsCount = params->columnsCount;

    const int64_t duration = mAnimation->getDuration();
    const float columnDelay = mColumnDelay * duration;
    const float rowDelay = mRowDelay * duration;

    float totalDelay;
    int64_t viewDelay;

    if (mInterpolator == nullptr) {
        mInterpolator = new LinearInterpolator();
    }

    switch (mDirectionPriority) {
    case PRIORITY_COLUMN:
        viewDelay = (int64_t) (row * rowDelay + column * rowsCount * rowDelay);
        totalDelay = rowsCount * rowDelay + columnsCount * rowsCount * rowDelay;
        break;
    case PRIORITY_ROW:
        viewDelay = (int64_t) (column * columnDelay + row * columnsCount * columnDelay);
        totalDelay = columnsCount * columnDelay + rowsCount * columnsCount * columnDelay;
        break;
    case PRIORITY_NONE:
    default:
        viewDelay = (int64_t) (column * columnDelay + row * rowDelay);
        totalDelay = columnsCount * columnDelay + rowsCount * rowDelay;
        break;
    }

    float normalizedDelay = viewDelay / totalDelay;
    normalizedDelay = mInterpolator->getInterpolation(normalizedDelay);

    return normalizedDelay * totalDelay;
}

int GridLayoutAnimationController::getTransformedColumnIndex(const AnimationParameters* params){
    int index;
    switch (getOrder()) {
    case ORDER_REVERSE:
        index = params->columnsCount - 1 - params->column;
        break;
    case ORDER_RANDOM:
#if defined(__linux__)||defined(__unix__)
        //if (mRandomizer == null) mRandomizer = new Random();
        index = static_cast<int> (params->columnsCount * drand48());//mRandomizer.nextFloat());
#else
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 1.0);
            index = static_cast<int>(params->columnsCount * dis(gen));
        }
#endif
        break;
    case ORDER_NORMAL:
    default:
        index = params->column;
        break;
    }

    int direction = mDirection & DIRECTION_HORIZONTAL_MASK;
    if (direction == DIRECTION_RIGHT_TO_LEFT) {
        index = params->columnsCount - 1 - index;
    }
    return index;
}

int GridLayoutAnimationController::getTransformedRowIndex(const AnimationParameters* params){
    int index;
    switch (getOrder()) {
    case ORDER_REVERSE:
        index = params->rowsCount - 1 - params->row;
        break;
    case ORDER_RANDOM:
#if defined(__linux__)||defined(__unix__)
        //if (mRandomizer == null) mRandomizer = new Random();
        index = (int) (params->rowsCount * drand48());//mRandomizer.nextFloat());
#else
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 1.0);
            index = static_cast<int>(params->rowsCount * dis(gen));
        }

#endif
        break;
    case ORDER_NORMAL:
    default:
        index = params->row;
        break;
    }

    int direction = mDirection & DIRECTION_VERTICAL_MASK;
    if (direction == DIRECTION_BOTTOM_TO_TOP) {
        index = params->rowsCount - 1 - index;
    }
    return index;
}

}
