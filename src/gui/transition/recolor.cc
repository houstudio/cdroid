/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.Recolor.
 *********************************************************************************/
#include <transition/recolor.h>

#include <animation/objectanimator.h>
#include <animation/property.h>
#include <core/any.h>
#include <drawable/colordrawable.h>
#include <drawable/drawable.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <widget/textview.h>

namespace cdroid{

namespace {

// android animates by property name ("color" / "textColor"). CDROID uses explicit Property
// objects (the by-name form resolves through a different path); behavior is identical.
class ColorProperty: public Property{
public:
    ColorProperty(): Property("color", COLOR_TYPE){}
    AnimateValue get(void* object) const override{ return ((ColorDrawable*)object)->getColor(); }
    void set(void* object, const AnimateValue& v) const override{ ((ColorDrawable*)object)->setColor(GET_VARIANT(v, int)); }
};
ColorProperty COLOR_PROPERTY;

class TextColorProperty: public Property{
public:
    TextColorProperty(): Property("textColor", COLOR_TYPE){}
    AnimateValue get(void* object) const override{ return ((TextView*)object)->getCurrentTextColor(); }
    void set(void* object, const AnimateValue& v) const override{ ((TextView*)object)->setTextColor(GET_VARIANT(v, int)); }
};
TextColorProperty TEXT_COLOR_PROPERTY;

} // anonymous namespace

void Recolor::captureValues(TransitionValues& transitionValues){
    transitionValues.values[PROPNAME_BACKGROUND] = transitionValues.view->getBackground();
    if (TextView* tv = dynamic_cast<TextView*>(transitionValues.view)){
        transitionValues.values[PROPNAME_TEXT_COLOR] = tv->getCurrentTextColor();
    }
}

void Recolor::captureStartValues(TransitionValues& transitionValues){
    captureValues(transitionValues);
}

void Recolor::captureEndValues(TransitionValues& transitionValues){
    captureValues(transitionValues);
}

Animator* Recolor::createAnimator(ViewGroup* /*sceneRoot*/,
        TransitionValues* startValues, TransitionValues* endValues){
    if (startValues == nullptr || endValues == nullptr){
        return nullptr;
    }
    View* view = endValues->view;
    Drawable* startBackground = nonstd::any_cast<Drawable*>(startValues->values.at(PROPNAME_BACKGROUND));
    Drawable* endBackground = nonstd::any_cast<Drawable*>(endValues->values.at(PROPNAME_BACKGROUND));

    if (startBackground && endBackground){
        ColorDrawable* startColor = dynamic_cast<ColorDrawable*>(startBackground);
        ColorDrawable* endColor = dynamic_cast<ColorDrawable*>(endBackground);
        if (startColor && endColor && startColor->getColor() != endColor->getColor()){
            endColor->setColor(startColor->getColor());
            return ObjectAnimator::ofArgb(endBackground, &COLOR_PROPERTY,
                    {startColor->getColor(), endColor->getColor()});
        }
    }

    if (TextView* textView = dynamic_cast<TextView*>(view)){
        // PROPNAME_TEXT_COLOR is only present for TextViews (both start and end).
        if (startValues->values.count(PROPNAME_TEXT_COLOR) && endValues->values.count(PROPNAME_TEXT_COLOR)){
            int start = nonstd::any_cast<int>(startValues->values.at(PROPNAME_TEXT_COLOR));
            int end   = nonstd::any_cast<int>(endValues->values.at(PROPNAME_TEXT_COLOR));
            if (start != end){
                textView->setTextColor(end);
                return ObjectAnimator::ofArgb(textView, &TEXT_COLOR_PROPERTY, {start, end});
            }
        }
    }
    return nullptr;
}

} // namespace cdroid
