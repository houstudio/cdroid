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
#include <drawable/drawables.h>
#include <animation/valueanimator.h>
#include <drawable/animationscalelistdrawable.h>
namespace cdroid{
AnimationScaleListDrawable::AnimationScaleListDrawable():AnimationScaleListDrawable(nullptr){
}

AnimationScaleListDrawable::AnimationScaleListDrawable(std::shared_ptr<AnimationScaleListState> state) {
    // Every scale list drawable has its own constant state.
    auto newState = std::make_shared<AnimationScaleListState>(state.get(), this);
    setConstantState(newState);
    onStateChange(getState());
}

/**
 * Set the current drawable according to the animation scale. If scale is 0, then pick the
 * static drawable, otherwise, pick the animatable drawable.
 */
bool AnimationScaleListDrawable::onStateChange(const std::vector<int>& stateSet){
    const bool changed = DrawableContainer::onStateChange(stateSet);
    const int idx = mAnimationScaleListState->getCurrentDrawableIndexBasedOnScale();
    return selectDrawable(idx) || changed;
}


void AnimationScaleListDrawable::inflate(XmlPullParser& parser,const AttributeSet& attrs){
    //updateDensity();
    inflateChildElements(parser, attrs);
    onStateChange(getState());
}

/**
 * Inflates child elements from XML.
 */
void AnimationScaleListDrawable::inflateChildElements(XmlPullParser& parser,const AttributeSet& attrs){
    auto state = mAnimationScaleListState;
    int type, depth;
    const int innerDepth = parser.getDepth()+1;
    while ((type = parser.next()) != XmlPullParser::END_DOCUMENT
            && ((depth=parser.getDepth()) >= innerDepth || type != XmlPullParser::END_TAG)) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        if ((depth > innerDepth) || parser.getName().compare("item")) {
            continue;
        }

        // Either pick up the android:drawable attribute.
        Drawable* dr = attrs.getDrawable("drawable");

        // Or parse the child element under <item>.
        if (dr == nullptr) {
            while ((type = parser.next()) == XmlPullParser::TEXT) {
            }
            if (type != XmlPullParser::START_TAG) {
                throw std::logic_error(parser.getPositionDescription()+
                                ": <item> tag requires a 'drawable' attribute or "
                                "child tag defining a drawable");
            }
            dr = Drawable::createFromXmlInner(parser,attrs);
        }
        state->addDrawable(dr);
    }
}

AnimationScaleListDrawable* AnimationScaleListDrawable::mutate(){
    if (!mMutated && DrawableContainer::mutate() == this) {
        mAnimationScaleListState->mutate();
        mMutated = true;
    }
    return this;
}

void AnimationScaleListDrawable::clearMutated(){
    DrawableContainer::clearMutated();
    mMutated = false;
}

void AnimationScaleListDrawable::start(){
    Drawable* dr = getCurrent();
    if ((dr != nullptr) && dynamic_cast<Animatable*>(dr)) {
        ((Animatable*) dr)->start();
        if(dynamic_cast<AnimatedImageDrawable*>(dr))
            ((AnimatedImageDrawable*)dr)->start();
        else if(dynamic_cast<AnimatedRotateDrawable*>(dr))
            ((AnimatedRotateDrawable*)dr)->start();
        else if(dynamic_cast<AnimationDrawable*>(dr))
            ((AnimationDrawable*)dr)->start();
        else if(dynamic_cast<AnimatedVectorDrawable*>(dr))
            ((AnimatedVectorDrawable*)dr)->start();
    }
}

void AnimationScaleListDrawable::stop(){
    Drawable* dr = getCurrent();
    if ((dr != nullptr) && dynamic_cast<Animatable*>(dr)) {
        if(dynamic_cast<AnimationDrawable*>(dr))
            ((AnimationDrawable*)dr)->stop();
        else if(dynamic_cast<AnimatedRotateDrawable*>(dr))
            ((AnimatedRotateDrawable*)dr)->stop();
        else if(dynamic_cast<AnimatedImageDrawable*>(dr))
            ((AnimatedImageDrawable*)dr)->stop();
        else if(dynamic_cast<AnimatedVectorDrawable*>(dr))
            ((AnimatedVectorDrawable*)dr)->stop();
    }
}

bool AnimationScaleListDrawable::isRunning(){
    bool result = false;
    Drawable* dr = getCurrent();
    if ((dr != nullptr) && dynamic_cast<Animatable*>(dr)) {
        if(dynamic_cast<AnimationDrawable*>(dr))
            result = ((AnimationDrawable*)dr)->isRunning();
        else if(dynamic_cast<AnimatedRotateDrawable*>(dr))
            result = ((AnimatedRotateDrawable*)dr)->isRunning();
        else if(dynamic_cast<AnimatedImageDrawable*>(dr))
            result = ((AnimatedImageDrawable*)dr)->isRunning();
        else if(dynamic_cast<AnimatedVectorDrawable*>(dr))
            result = ((AnimatedVectorDrawable*)dr)->isRunning();
    }
    return result;
}

/*void AnimationScaleListDrawable::applyTheme(Theme theme) {
    DrawableContainer::applyTheme(theme);
    onStateChange(getState());
}*/

void AnimationScaleListDrawable::setConstantState(std::shared_ptr<DrawableContainerState> state){
    DrawableContainer::setConstantState(state);

    if (dynamic_cast<AnimationScaleListState*>(state.get())) {
        mAnimationScaleListState = std::dynamic_pointer_cast<AnimationScaleListState>(state);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnimationScaleListDrawable::AnimationScaleListState::AnimationScaleListState(const AnimationScaleListState* orig, AnimationScaleListDrawable* owner)
    :DrawableContainerState(orig, owner){

    if (orig != nullptr) {
        // Perform a shallow copy and rely on mutate() to deep-copy.
        //mThemeAttrs = orig->mThemeAttrs;

        mStaticDrawableIndex = orig->mStaticDrawableIndex;
        mAnimatableDrawableIndex = orig->mAnimatableDrawableIndex;
    }

}

void AnimationScaleListDrawable::AnimationScaleListState::mutate() {
    //mThemeAttrs = mThemeAttrs != null ? mThemeAttrs.clone() : null;
}

/**
 * Add the drawable into the container.
 * This class only keep track one animatable drawable, and one static. If there are multiple
 * defined in the XML, then pick the last one.
 */
int AnimationScaleListDrawable::AnimationScaleListState::addDrawable(Drawable* drawable) {
    const int pos = addChild(drawable);
    if (dynamic_cast<Animatable*>(drawable)) {
        mAnimatableDrawableIndex = pos;
    } else {
        mStaticDrawableIndex = pos;
    }
    return pos;
}

AnimationScaleListDrawable* AnimationScaleListDrawable::AnimationScaleListState::newDrawable(){
    return new AnimationScaleListDrawable(std::dynamic_pointer_cast<AnimationScaleListState>(shared_from_this()));
}

/*bool AnimationScaleListDrawable::AnimationScaleListState::canApplyTheme() {
    return mThemeAttrs != nullptr || DrawableContainerState::canApplyTheme();
}*/

int AnimationScaleListDrawable::AnimationScaleListState::getCurrentDrawableIndexBasedOnScale() {
    if (ValueAnimator::getDurationScale() == 0) {
        return mStaticDrawableIndex;
    }
    return mAnimatableDrawableIndex;
}

}/*endof namespace*/
