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

namespace cdroid {

TransitionInflater* TransitionInflater::from(Context* context) {
    return new TransitionInflater(context);
}

Transition* TransitionInflater::inflateTransition(const std::string& resource) {
    XmlPullParser parser(mContext, resource);
    return createTransitionFromXml(parser, nullptr);
}

TransitionManager* TransitionInflater::inflateTransitionManager(const std::string& resource, ViewGroup* sceneRoot) {
    XmlPullParser parser(mContext, resource);
    return createTransitionManagerFromXml(parser, sceneRoot);
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
            std::string v;
            v = parser.getAttributeValue("targetId");
            if (!v.empty()) {
                transition->addTarget(atoi(v.c_str()));
                continue;
            }
            v = parser.getAttributeValue("excludeId");
            if (!v.empty()) {
                transition->excludeTarget(atoi(v.c_str()), true);
                continue;
            }
            v = parser.getAttributeValue("targetName");
            if (!v.empty()) {
                transition->addTarget(v);
                continue;
            }
            v = parser.getAttributeValue("excludeName");
            if (!v.empty()) {
                transition->excludeTarget(v, true);
                continue;
            }
            // targetClass/excludeClass need Class/reflection — skipped (CDROID has no reflection).
            v = parser.getAttributeValue("targetClass");
            if (!v.empty()) {
                LOGW("TransitionInflater: targetClass '%s' not supported (no reflection)", v.c_str());
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
    // android resolves transition/fromScene/toScene by int resource id. CDROID is string-based;
    // Scene.getSceneForLayout takes an int layoutId (the layoutId inflation path is itself
    // stubbed — see Scene). Resolve what we can; scene-based TransitionManager XML is limited.
    std::string transitionRef = attrs.getAttributeValue("transition");
    std::string toSceneRef = attrs.getAttributeValue("toScene");
    if (!transitionRef.empty() && !toSceneRef.empty()) {
        Transition* transition = inflateTransition(transitionRef);
        if (transition != nullptr) {
            // toScene/fromScene are layout resources; Scene layoutId path is stubbed, so use a
            // plain Scene(sceneRoot) and setTransition(toScene). Best-effort.
            Scene* toScene = new Scene(sceneRoot);
            tm->setTransition(toScene, transition);
        }
    }
}

} // namespace cdroid
