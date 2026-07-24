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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *********************************************************************************/
#include <transition/changetext.h>

#include <animation/animator.h>
#include <animation/animatorset.h>
#include <animation/valueanimator.h>
#include <core/any.h>
#include <porting/cdlog.h>
#include <view/viewgroup.h>
#include <widget/edittext.h>
#include <widget/textview.h>

#include <transition/transitionlisteneradapter.h>

namespace cdroid{

namespace {

constexpr const char* LOG_TAG = "TextChange";
constexpr bool DBG = false;

void setSelection(EditText* editText, int start, int end){
    if (start >= 0 && end >= 0){
        editText->setSelection(start, end);
    }
}

// android: anonymous TransitionListenerAdapter inside createAnimator. Named here
// (C++ has no anonymous classes) — restores text/selection/color on pause/resume/end.
struct ChangeTextListener: public TransitionListenerAdapter{
    TextView* view;
    std::string startText;
    std::string endText;
    int startSelectionStart;
    int startSelectionEnd;
    int endSelectionStart;
    int endSelectionEnd;
    int startColor;
    int endColor;
    int changeBehavior;
    int mPausedColor = 0;

    void onTransitionPause(Transition& /*transition*/) override{
        if (changeBehavior != ChangeText::CHANGE_BEHAVIOR_IN){
            view->setText(endText);   // setText(string) — see deviation note in header
            if (EditText* et = dynamic_cast<EditText*>(view)){
                setSelection(et, endSelectionStart, endSelectionEnd);
            }
        }
        if (changeBehavior > ChangeText::CHANGE_BEHAVIOR_KEEP){
            mPausedColor = view->getCurrentTextColor();
            view->setTextColor(endColor);
        }
    }

    void onTransitionResume(Transition& /*transition*/) override{
        if (changeBehavior != ChangeText::CHANGE_BEHAVIOR_IN){
            view->setText(startText);
            if (EditText* et = dynamic_cast<EditText*>(view)){
                setSelection(et, startSelectionStart, startSelectionEnd);
            }
        }
        if (changeBehavior > ChangeText::CHANGE_BEHAVIOR_KEEP){
            view->setTextColor(mPausedColor);
        }
    }

    void onTransitionEnd(Transition& transition) override{
        transition.removeListener(this); // Transition frees it at destruction
    }
};

} // anonymous namespace

const std::vector<std::string> ChangeText::sTransitionProperties = {
    PROPNAME_TEXT, PROPNAME_TEXT_SELECTION_START, PROPNAME_TEXT_SELECTION_END};

ChangeText& ChangeText::setChangeBehavior(int changeBehavior){
    if (changeBehavior >= CHANGE_BEHAVIOR_KEEP && changeBehavior <= CHANGE_BEHAVIOR_OUT_IN){
        mChangeBehavior = changeBehavior;
    }
    return *this;
}

std::vector<std::string> ChangeText::getTransitionProperties(){
    return sTransitionProperties;
}

void ChangeText::captureValues(TransitionValues& transitionValues){
    if (TextView* textview = dynamic_cast<TextView*>(transitionValues.view)){
        transitionValues.values[PROPNAME_TEXT] = textview->getText().toUTF8(); // snapshot
        if (dynamic_cast<EditText*>(transitionValues.view)){
            transitionValues.values[PROPNAME_TEXT_SELECTION_START] = textview->getSelectionStart();
            transitionValues.values[PROPNAME_TEXT_SELECTION_END]   = textview->getSelectionEnd();
        }
        if (mChangeBehavior > CHANGE_BEHAVIOR_KEEP){
            transitionValues.values[PROPNAME_TEXT_COLOR] = textview->getCurrentTextColor();
        }
    }
}

void ChangeText::captureStartValues(TransitionValues& transitionValues){
    captureValues(transitionValues);
}

void ChangeText::captureEndValues(TransitionValues& transitionValues){
    captureValues(transitionValues);
}

Animator* ChangeText::createAnimator(ViewGroup* /*sceneRoot*/,
        TransitionValues* startValues, TransitionValues* endValues){
    if (startValues == nullptr || endValues == nullptr ||
            dynamic_cast<TextView*>(startValues->view) == nullptr ||
            dynamic_cast<TextView*>(endValues->view) == nullptr){
        return nullptr;
    }
    TextView* view = dynamic_cast<TextView*>(endValues->view);
    std::map<std::string, nonstd::any>& startVals = startValues->values;
    std::map<std::string, nonstd::any>& endVals   = endValues->values;

    auto getString = [](std::map<std::string, nonstd::any>& m, const char* key) -> std::string{
        auto it = m.find(key);
        return (it != m.end()) ? nonstd::any_cast<std::string>(it->second) : std::string();
    };
    auto getInt = [](std::map<std::string, nonstd::any>& m, const char* key, int def) -> int{
        auto it = m.find(key);
        return (it != m.end()) ? nonstd::any_cast<int>(it->second) : def;
    };

    const std::string startText = getString(startVals, PROPNAME_TEXT);
    const std::string endText   = getString(endVals, PROPNAME_TEXT);

    int startSelectionStart, startSelectionEnd, endSelectionStart, endSelectionEnd;
    if (dynamic_cast<EditText*>(view)){
        startSelectionStart = getInt(startVals, PROPNAME_TEXT_SELECTION_START, -1);
        startSelectionEnd   = getInt(startVals, PROPNAME_TEXT_SELECTION_END, startSelectionStart);
        endSelectionStart   = getInt(endVals, PROPNAME_TEXT_SELECTION_START, -1);
        endSelectionEnd     = getInt(endVals, PROPNAME_TEXT_SELECTION_END, endSelectionStart);
    } else {
        startSelectionStart = startSelectionEnd = endSelectionStart = endSelectionEnd = -1;
    }

    if (startText != endText){
        int startColor;
        int endColor;
        if (mChangeBehavior != CHANGE_BEHAVIOR_IN){
            view->setText(startText);
            if (EditText* et = dynamic_cast<EditText*>(view)){
                setSelection(et, startSelectionStart, startSelectionEnd);
            }
        }
        Animator* anim;
        if (mChangeBehavior == CHANGE_BEHAVIOR_KEEP){
            startColor = endColor = 0;
            anim = ValueAnimator::ofFloat({0.0f, 1.0f});
            Animator::AnimatorListener listener;
            listener.onAnimationEnd = [view, endText, endSelectionStart, endSelectionEnd, startText](Animator&, bool){
                if (startText == view->getText().toUTF8()){
                    // Only set if it hasn't been changed since anim started
                    view->setText(endText);
                    if (EditText* et = dynamic_cast<EditText*>(view)){
                        setSelection(et, endSelectionStart, endSelectionEnd);
                    }
                }
            };
            anim->addListener(listener);
        } else {
            startColor = getInt(startVals, PROPNAME_TEXT_COLOR, 0);
            endColor   = getInt(endVals, PROPNAME_TEXT_COLOR, 0);
            // Fade out start text
            ValueAnimator* outAnim = nullptr;
            ValueAnimator* inAnim  = nullptr;
            if (mChangeBehavior == CHANGE_BEHAVIOR_OUT_IN || mChangeBehavior == CHANGE_BEHAVIOR_OUT){
                outAnim = ValueAnimator::ofInt({(startColor >> 24) & 0xff, 0}); // Color.alpha(startColor)
                outAnim->addUpdateListener([view, startColor](ValueAnimator& animation){
                    int currAlpha = nonstd::any_cast<int>(animation.getAnimatedValue());
                    view->setTextColor((currAlpha << 24) | (startColor & 0xffffff));
                });
                Animator::AnimatorListener outListener;
                outListener.onAnimationEnd = [view, endText, endSelectionStart, endSelectionEnd, startText, endColor](Animator&, bool){
                    if (startText == view->getText().toUTF8()){
                        view->setText(endText);
                        if (EditText* et = dynamic_cast<EditText*>(view)){
                            setSelection(et, endSelectionStart, endSelectionEnd);
                        }
                    }
                    // restore opaque alpha and correct end color
                    view->setTextColor(endColor);
                };
                outAnim->addListener(outListener);
            }
            if (mChangeBehavior == CHANGE_BEHAVIOR_OUT_IN || mChangeBehavior == CHANGE_BEHAVIOR_IN){
                inAnim = ValueAnimator::ofInt({0, (endColor >> 24) & 0xff}); // Color.alpha(endColor)
                inAnim->addUpdateListener([view, endColor](ValueAnimator& animation){
                    int currAlpha = nonstd::any_cast<int>(animation.getAnimatedValue());
                    view->setTextColor((currAlpha << 24) | (endColor & 0xffffff));
                });
                Animator::AnimatorListener inListener;
                inListener.onAnimationCancel = [view, endColor](Animator&){
                    // restore opaque alpha and correct end color
                    view->setTextColor(endColor);
                };
                inAnim->addListener(inListener);
            }
            if (outAnim != nullptr && inAnim != nullptr){
                AnimatorSet* set = new AnimatorSet();
                set->playSequentially({outAnim, inAnim});
                anim = set;
            } else if (outAnim != nullptr){
                anim = outAnim;
            } else {
                // Must be an in-only animation
                anim = inAnim;
            }
        }
        ChangeTextListener* transitionListener = new ChangeTextListener();
        transitionListener->view = view;
        transitionListener->startText = startText;
        transitionListener->endText = endText;
        transitionListener->startSelectionStart = startSelectionStart;
        transitionListener->startSelectionEnd = startSelectionEnd;
        transitionListener->endSelectionStart = endSelectionStart;
        transitionListener->endSelectionEnd = endSelectionEnd;
        transitionListener->startColor = startColor;
        transitionListener->endColor = endColor;
        transitionListener->changeBehavior = mChangeBehavior;
        addListener(transitionListener); // Transition takes ownership
        if (DBG){
            LOGD("%s: createAnimator returning %p", LOG_TAG, (void*)anim);
        }
        return anim;
    }
    return nullptr;
}

} // namespace cdroid
