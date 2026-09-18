/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.TransitionInflater.
 *
 * Resource note: resolves by int resource id (AOSP @TransitionRes), same as
 * android. Custom <transition class="..."> uses java reflection, which CDROID lacks —
 * that path throws (custom transitions must be constructed in code).
 *********************************************************************************/
#ifndef __CDROID_TRANSITION_TRANSITIONINFLATER_H__
#define __CDROID_TRANSITION_TRANSITIONINFLATER_H__

#include <string>

#include <transition/transition.h>

namespace cdroid {

class Context;
class ViewGroup;
class TransitionManager;
class XmlPullParser;
class AttributeSet;
class PathMotion;
class Scene;

/**
 * Inflates scenes and transitions from XML resource files.
 * Ported from android-36 android.transition.TransitionInflater.
 */
class TransitionInflater {
  public:
    static TransitionInflater* from(Context* context);

    Transition* inflateTransition(int resourceId);
    TransitionManager* inflateTransitionManager(int resourceId, ViewGroup* sceneRoot);

  private:
    explicit TransitionInflater(Context* context): mContext(context) {}
    Transition* createTransitionFromXml(XmlPullParser& parser, Transition* parent);
    void getTargetIds(XmlPullParser& parser, Transition* transition);
    TransitionManager* createTransitionManagerFromXml(XmlPullParser& parser, ViewGroup* sceneRoot);
    void loadTransition(const AttributeSet& attrs, ViewGroup* sceneRoot, TransitionManager* tm);

    Context* mContext;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_TRANSITIONINFLATER_H__
