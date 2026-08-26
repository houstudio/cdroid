/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.TransitionInflater.
 *********************************************************************************/
#include <transition/transitioninflater.h>

#include <cstdlib>
#include <stdexcept>

#include <core/xmlpullparser.h>
#include <porting/cdlog.h>
#include <view/viewgroup.h>

#include <transition/arcmotion.h>
#include <transition/autotransition.h>
#include <transition/changebounds.h>
#include <transition/changeclipbounds.h>
#include <transition/changeimagetransform.h>
#include <transition/changescroll.h>
#include <transition/crossfade.h>
#include <transition/explode.h>
#include <transition/fade.h>
#include <transition/patternpathmotion.h>
#include <transition/recolor.h>
#include <transition/scene.h>
#include <transition/slide.h>
#include <transition/transitionmanager.h>
#include <transition/transitionset.h>
#include <content/typedarray.h>
#include <widget/framework_styleable.h>

namespace cdroid {

TransitionInflater* TransitionInflater::from(Context* context) {
    return new TransitionInflater(context);
}

Transition* TransitionInflater::inflateTransition(int resourceId) {
    auto parser = mContext->getResources().getXml(resourceId);
    return createTransitionFromXml(*parser, nullptr);
}

Transition* TransitionInflater::inflateTransition(const std::string& resource) {
    auto parser = mContext->getResources().getXml(resource);
    return createTransitionFromXml(*parser, nullptr);
}

TransitionManager* TransitionInflater::inflateTransitionManager(int resourceId, ViewGroup* sceneRoot) {
    auto parser = mContext->getResources().getXml(resourceId);
    return createTransitionManagerFromXml(*parser, sceneRoot);
}

TransitionManager* TransitionInflater::inflateTransitionManager(const std::string& resource, ViewGroup* sceneRoot) {
    auto parser = mContext->getResources().getXml(resource);
    return createTransitionManagerFromXml(*parser, sceneRoot);
}

Transition* TransitionInflater::createTransitionFromXml(XmlPullParser& parser, Transition* parent) {
    Transition* transition = nullptr;
    int type;
    int depth = parser.getDepth();
    TransitionSet* transitionSet = dynamic_cast<TransitionSet*>(parent);

    while (((type = parser.next()) != XmlPullParser::END_TAG || parser.getDepth() > depth)
            && type != XmlPullParser::END_DOCUMENT) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }
        std::string name = parser.getName();
        if (name == "fade") {
            transition = new Fade(mContext, &parser);
        } else if (name == "changeBounds") {
            transition = new ChangeBounds(mContext, &parser);
        } else if (name == "slide") {
            transition = new Slide(mContext, &parser);
        } else if (name == "explode") {
            transition = new Explode(mContext, &parser);
        } else if (name == "changeImageTransform") {
            transition = new ChangeImageTransform(mContext, &parser);
        } else if (name == "changeClipBounds") {
            transition = new ChangeClipBounds(mContext, &parser);
        } else if (name == "autoTransition") {
            transition = new AutoTransition(mContext, &parser);
        } else if (name == "recolor") {
            transition = new Recolor(mContext, &parser);
        } else if (name == "changeScroll") {
            transition = new ChangeScroll(mContext, &parser);
        } else if (name == "transitionSet") {
            transition = new TransitionSet(mContext, &parser);
        } else if (name == "transition") {
            // android: createCustom(attrs, Transition.class, "transition") via reflection.
            LOGE("TransitionInflater: custom <transition class=\"...\"> not supported (no reflection).");
            throw std::runtime_error("Custom transition class not supported in CDROID; build in code.");
        } else if (name == "targets") {
            getTargetIds(parser, parent);
            continue;
        } else if (name == "arcMotion") {
            if (parent != nullptr) {
                parent->setPathMotion(new ArcMotion(mContext, &parser));
            }
            continue;
        } else if (name == "pathMotion") {
            LOGE("TransitionInflater: custom <pathMotion class=\"...\"> not supported (no reflection).");
            throw std::runtime_error("Custom pathMotion class not supported in CDROID.");
        } else if (name == "patternPathMotion") {
            if (parent != nullptr) {
                parent->setPathMotion(new PatternPathMotion(mContext, &parser));
            }
            continue;
        } else {
            throw std::runtime_error("Unknown scene name: " + name);
        }
        if (transition != nullptr) {
            createTransitionFromXml(parser, transition); // recurse into children (empty tags return null)
            if (transitionSet != nullptr) {
                transitionSet->addTransition(transition);
                transition = nullptr;
            } else if (parent != nullptr) {
                throw std::runtime_error("Could not add transition to another transition.");
            }
        }
    }
    return transition;
}

void TransitionInflater::getTargetIds(XmlPullParser& parser, Transition* transition) {
    int type;
    int depth = parser.getDepth();
    while (((type = parser.next()) != XmlPullParser::END_TAG || parser.getDepth() > depth)
            && type != XmlPullParser::END_DOCUMENT) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }
        std::string name = parser.getName();
        if (name == "target") {
            auto a = mContext->obtainStyledAttributes(&parser,
                    internal::R::styleable::TransitionTarget);
            int id = a->getResourceId(internal::R::styleable::TransitionTarget_targetId, 0);
            std::string transitionName;
            if (id != 0) {
                transition->addTarget(id);
            } else if ((id = a->getResourceId(
                    internal::R::styleable::TransitionTarget_excludeId, 0)) != 0) {
                transition->excludeTarget(id, true);
            } else if (!(transitionName = a->getString(
                    internal::R::styleable::TransitionTarget_targetName)).empty()) {
                transition->addTarget(transitionName);
            } else if (!(transitionName = a->getString(
                    internal::R::styleable::TransitionTarget_excludeName)).empty()) {
                transition->excludeTarget(transitionName, true);
            } else {
                // excludeClass/targetClass need Class/reflection — CDROID has no reflection.
                std::string className = a->getString(
                        internal::R::styleable::TransitionTarget_excludeClass);
                if (!className.empty()) {
                    LOGW("TransitionInflater: excludeClass '%s' not supported (no reflection)",
                         className.c_str());
                } else if (!(className = a->getString(
                        internal::R::styleable::TransitionTarget_targetClass)).empty()) {
                    LOGW("TransitionInflater: targetClass '%s' not supported (no reflection)",
                         className.c_str());
                }
            }
        } else {
            throw std::runtime_error("Unknown scene name: " + name);
        }
    }
}

TransitionManager* TransitionInflater::createTransitionManagerFromXml(XmlPullParser& parser, ViewGroup* sceneRoot) {
    int type;
    int depth = parser.getDepth();
    TransitionManager* transitionManager = nullptr;
    while (((type = parser.next()) != XmlPullParser::END_TAG || parser.getDepth() > depth)
            && type != XmlPullParser::END_DOCUMENT) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }
        std::string name = parser.getName();
        if (name == "transitionManager") {
            transitionManager = new TransitionManager();
        } else if (name == "transition" && transitionManager != nullptr) {
            loadTransition(parser, sceneRoot, transitionManager);
        } else {
            throw std::runtime_error("Unknown scene name: " + name);
        }
    }
    return transitionManager;
}

void TransitionInflater::loadTransition(const AttributeSet& attrs, ViewGroup* sceneRoot, TransitionManager* tm) {
    auto a = mContext->obtainStyledAttributes(&attrs, internal::R::styleable::TransitionManager);
    const int transitionId = a->getResourceId(internal::R::styleable::TransitionManager_transition, -1);
    const int fromId = a->getResourceId(internal::R::styleable::TransitionManager_fromScene, -1);
    const int toId = a->getResourceId(internal::R::styleable::TransitionManager_toScene, -1);
    (void)fromId;
    if (transitionId >= 0 && toId >= 0) {
        Transition* transition = inflateTransition(transitionId);
        if (transition != nullptr) {
            // toScene/fromScene are layout resources; the Scene layoutId path is stubbed, so a
            // plain Scene(sceneRoot) stands in for toScene. Best-effort.
            Scene* toScene = new Scene(sceneRoot);
            tm->setTransition(toScene, transition);
        }
    }
}

} // namespace cdroid
