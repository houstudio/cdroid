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
*/

#include <widgetEx/wear/circularprogresslayout.h>
#include <widgetEx/widgetex_styleable.h>

#include <widgetEx/wear/circularprogressdrawable.h>
#include <widgetEx/wear/circularprogresslayoutcontroller.h>
#include <cairomm/context.h>
namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET2(CircularProgressLayout, "androidx.wear.widget.CircularProgressLayout");

CircularProgressLayout::CircularProgressLayout(Context* context,const AttributeSet* attrs):CircularProgressLayout(context,attrs,0){}

CircularProgressLayout::CircularProgressLayout(Context* context,const AttributeSet* pAttrs,int defStyleAttr)
    :FrameLayout(context, pAttrs, defStyleAttr){

    initCircularProgressLayout();
    // androidx CircularProgressLayout.java:137-158.
    auto ta = context->obtainStyledAttributes(pAttrs, internal::R::styleable::CircularProgressLayout, defStyleAttr);
    if (ta) {
        if (ta->getResourceId(internal::R::styleable::CircularProgressLayout_colorSchemeColors, 0) != 0
                || !ta->hasValue(internal::R::styleable::CircularProgressLayout_colorSchemeColors)) {
            int arrayResId = (int)ta->getResourceId(
                    internal::R::styleable::CircularProgressLayout_colorSchemeColors, 0);
            if (arrayResId == 0) {
                arrayResId = (int)internal::R::array::circular_progress_layout_color_scheme_colors;
            }
            setColorSchemeColors(getColorListFromResources(arrayResId));
        } else {
            setColorSchemeColors({(int)ta->getColor(
                    internal::R::styleable::CircularProgressLayout_colorSchemeColors, Color::BLACK)});
        }
        const int defStroke = context->getResources().getDimensionPixelSize(
                (int)internal::R::dimen::circular_progress_layout_stroke_width);
        setStrokeWidth(ta->getDimensionPixelSize(
                internal::R::styleable::CircularProgressLayout_strokeWidth, defStroke));
        const int defBg = context->getColor((int)internal::R::color::circular_progress_layout_background_color);
        setBackgroundColor(ta->getColor(internal::R::styleable::CircularProgressLayout_backgroundColor, defBg));
        setIndeterminate(ta->getBoolean(internal::R::styleable::CircularProgressLayout_indeterminate, false));
    }
}

void CircularProgressLayout::initCircularProgressLayout(){
    mProgressDrawable = new CircularProgressDrawable(mContext);
    mProgressDrawable->setProgressRotation(DEFAULT_ROTATION);
    mProgressDrawable->setStrokeCap(static_cast<int>(Cairo::Context::LineCap::BUTT));
    setBackground(mProgressDrawable);

    OnHierarchyChangeListener hcl;
    hcl.onChildViewAdded=[](View&parent,View*child){
        // Ensure that child view is aligned in center
        LayoutParams* params = (LayoutParams*) child->getLayoutParams();
        params->gravity = Gravity::CENTER;
        child->setLayoutParams(params);
    };
    hcl.onChildViewRemoved=[](View&parent,View*child){
    };
    // If a child view is added, make it center aligned so it fits in the progress drawable.
    setOnHierarchyChangeListener(hcl);

    mController = new CircularProgressLayoutController(this);
}

CircularProgressLayout::~CircularProgressLayout(){
    /*mProgressDrawable is owned by View's Background,do not delete it*/
    delete mController;
}

std::vector<int> CircularProgressLayout::getColorListFromResources(int arrayResId) {
    // androidx CircularProgressLayout.java:165-171. A plain @color id (kept as
    // a reference by binary AXML) resolves here as a single-element color.
    std::vector<int> colors;
    if (arrayResId == 0) return colors;
    auto colorArray = mContext->getResources().obtainTypedArray(arrayResId);
    if (!colorArray) {
        const int color = mContext->getResources().getColor(arrayResId);
        if (color != 0) colors.push_back(color);
        return colors;
    }
    colors.reserve(colorArray->length());
    for (size_t i = 0; i < colorArray->length(); i++) {
        colors.push_back((int)colorArray->getColor(i, 0));
    }
    return colors;
}

void CircularProgressLayout::onLayout(bool changed, int left, int top, int right, int bottom) {
    FrameLayout::onLayout(changed, left, top, right, bottom);
    if (getChildCount() != 0) {
        View* childView = getChildAt(0);
        // Wrap the drawable around the child view
        mProgressDrawable->setCenterRadius(
                std::min(childView->getWidth(), childView->getHeight()) / 2.f);
    } else {
        // Fill the bounds if no child view is present
        mProgressDrawable->setCenterRadius(0.f);
    }
}

void CircularProgressLayout::onDetachedFromWindow() {
    FrameLayout::onDetachedFromWindow();
    mController->reset();
}

void CircularProgressLayout::setBackgroundColor(int color) {
    mProgressDrawable->setBackgroundColor(color);
}

int CircularProgressLayout::getBackgroundColor() {
    return mProgressDrawable->getBackgroundColor();
}

CircularProgressDrawable* CircularProgressLayout::getProgressDrawable() {
    return mProgressDrawable;
}

void CircularProgressLayout::setIndeterminate(bool indeterminate) {
    mController->setIndeterminate(indeterminate);
}

bool CircularProgressLayout::isIndeterminate() const{
    return mController->isIndeterminate();
}

void CircularProgressLayout::setTotalTime(int64_t totalTime) {
    if (totalTime <= 0) {
        throw std::invalid_argument("Total time should be greater than zero.");
    }
    mTotalTime = totalTime;
}

int64_t CircularProgressLayout::getTotalTime() const{
    return mTotalTime;
}

void CircularProgressLayout::startTimer() {
    mController->startTimer(mTotalTime, DEFAULT_UPDATE_INTERVAL);
    mProgressDrawable->setProgressRotation(mStartingRotation);
}

void CircularProgressLayout::stopTimer() {
    mController->stopTimer();
}

bool CircularProgressLayout::isTimerRunning() const{
    return mController->isTimerRunning();
}

void CircularProgressLayout::setStartingRotation(float rotation) {
    mStartingRotation = rotation;
}

float CircularProgressLayout::getStartingRotation() const{
    return mStartingRotation;
}

void CircularProgressLayout::setStrokeWidth(float strokeWidth) {
    mProgressDrawable->setStrokeWidth(strokeWidth);
}

float CircularProgressLayout::getStrokeWidth() const{
    return mProgressDrawable->getStrokeWidth();
}

void CircularProgressLayout::setColorSchemeColors(const std::vector<int>&colors) {
    mProgressDrawable->setColorSchemeColors(colors);
}

std::vector<int> CircularProgressLayout::getColorSchemeColors() const{
    return mProgressDrawable->getColorSchemeColors();
}

CircularProgressLayout::OnTimerFinishedListener CircularProgressLayout::getOnTimerFinishedListener() const{
    return mController->getOnTimerFinishedListener();
}

void CircularProgressLayout::setOnTimerFinishedListener(const OnTimerFinishedListener& listener) {
    mController->setOnTimerFinishedListener(listener);
}
}/*endof namespace*/
