#if 10
#include <animation/valueanimator.h>
#include <drawables/animationscalelistdrawable.h>
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


void AnimationScaleListDrawable::inflate(Context*context,XmlPullParser& parser,const AttributeSet& attrs){
    //updateDensity();
    inflateChildElements(parser, attrs);
    onStateChange(getState());
}

/**
 * Inflates child elements from XML.
 */
void AnimationScaleListDrawable::inflateChildElements(XmlPullParser& parser,const AttributeSet& attrs){
    auto state = mAnimationScaleListState;
    const int innerDepth = parser.getDepth() + 1;
    XmlPullParser::XmlEvent event;
    int type;
    int depth;
    while ((type = parser.next(event)) != XmlPullParser::END_DOCUMENT
            && ((depth = parser.getDepth()) >= innerDepth
            || type != XmlPullParser::END_TAG)) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        if (depth > innerDepth || !parser.getName().compare("item")==0) {
            continue;
        }

        // Either pick up the android:drawable attribute.
        Drawable* dr = attrs.getDrawable("drawable");

        // Or parse the child element under <item>.
        if (dr == nullptr) {
            while ((type = parser.next(event)) == XmlPullParser::TEXT) {
            }
            if (type != XmlPullParser::START_TAG) {
                throw std::logic_error(//parser.getPositionDescription()
                                ": <item> tag requires a 'drawable' attribute or "
                                "child tag defining a drawable");
            }
            dr = Drawable::createFromXmlInner(parser, attrs);
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
    if (dr != nullptr && dynamic_cast<Animatable*>(dr)) {
        ((Animatable*) dr)->start();
    }
}

void AnimationScaleListDrawable::stop(){
    Drawable* dr = getCurrent();
    if (dr != nullptr && dynamic_cast<Animatable*>(dr)) {
        ((Animatable*) dr)->stop();
    }
}

bool AnimationScaleListDrawable::isRunning(){
    bool result = false;
    Drawable* dr = getCurrent();
    if (dr != nullptr && dynamic_cast<Animatable*>(dr)) {
        result = ((Animatable*) dr)->isRunning();
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
    return new AnimationScaleListDrawable(std::dynamic_pointer_cast<AnimationScaleListState>(shared_from_this()));//, nullptr);
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
#endif
