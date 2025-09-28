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
#include <animation/gridlayoutanimationcontroller.h>

namespace cdroid{

GridLayoutAnimationController::GridLayoutAnimationController(Context* context,const AttributeSet& attrs)
  :LayoutAnimationController(context,attrs){
    mColumnDelay = attrs.getFloat("columnDelay");
    mRowDelay    = attrs.getFloat("rowDelay");
    mDirection   = attrs.getInt("direction",std::unordered_map<std::string,int>{
        {"left_to_right",(int)DIRECTION_LEFT_TO_RIGHT},
        {"right_to_left",(int)DIRECTION_RIGHT_TO_LEFT},
        {"top_to_bottom",(int)DIRECTION_TOP_TO_BOTTOM},
        {"bottom_to_top",(int)DIRECTION_BOTTOM_TO_TOP}
    }, DIRECTION_LEFT_TO_RIGHT | DIRECTION_TOP_TO_BOTTOM);
    mDirectionPriority = attrs.getInt("directionPriority",std::unordered_map<std::string,int>{
        {"none",(int)PRIORITY_NONE},
        {"column",(int)PRIORITY_COLUMN},
        {"row", (int)PRIORITY_ROW}
    },PRIORITY_NONE);
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
