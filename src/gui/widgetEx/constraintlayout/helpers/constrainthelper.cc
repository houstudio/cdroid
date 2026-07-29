/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.ConstraintHelper.
 */
#include <widgetEx/constraintlayout/helpers/constrainthelper.h>

#include <porting/cdlog.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <widgetEx/constraintlayout/constraintlayout.h>

namespace cdroid {

ConstraintHelper::ConstraintHelper(Context* ctx, const AttributeSet& attrs)
    : View(ctx, attrs) {
    init(attrs);
}

ConstraintHelper::ConstraintHelper(int width, int height)
    : View(width, height) {
}

void ConstraintHelper::init(const AttributeSet& attrs) {
    mReferenceIds = attrs.getString("constraint_referenced_ids", "");
    if (!mReferenceIds.empty()) {
        setIds(attrs,mReferenceIds);
    }
    // constraint_referenced_tags is deferred (CDROID LayoutParams has no constraintTag field yet).
}

void ConstraintHelper::addRscID(int id) {
    if (id == getId()) {
        return;
    }
    mIds.push_back(id);
}

void ConstraintHelper::addID(int id) {
    if (id != View::NO_ID) {
        addRscID(id);
    }
}

void ConstraintHelper::setIds(const AttributeSet& atts, const std::string& idList) {
    mReferenceIds = idList;
    if (idList.empty()) {
        return;
    }
    mIds.clear();
    auto trim = [](std::string s) -> std::string {
        auto notspace = [](unsigned char c) {
            return !std::isspace(c);
        };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), notspace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notspace).base(), s.end());
        return s;
    };
    size_t begin = 0;
    while (true) {
        size_t end = idList.find(',', begin);
        std::string token = trim((end == std::string::npos)
                                 ? idList.substr(begin)
                                 : idList.substr(begin, end - begin));
        if (!token.empty()) {
            // Android's constraint_referenced_ids holds bare names ("btn1, btn2"), resolved via
            // Resources.getIdentifier(name, "id", pkg). Context::getId is the CDROID equivalent;
            // the "id/" type prefix plays the role of the "id" type argument so a bare name
            // resolves (returns NO_ID/-1 on miss).
            std::string idname = std::string("id/") + token;
            int id = atts.getContext()->getId(idname);
            if (id == View::NO_ID) {
                LOGW("ConstraintHelper: could not resolve referenced id \"%s\"", token.c_str());
            }
            addID(id);
        }
        if (end == std::string::npos) {
            break;
        }
        begin = end + 1;
    }
}

std::vector<int> ConstraintHelper::getReferencedIds() const {
    return mIds;
}

void ConstraintHelper::setReferencedIds(const std::vector<int>& ids) {
    mReferenceIds.clear();
    mIds.clear();
    for (int id : ids) {
        addRscID(id);
    }
}

void ConstraintHelper::addView(View* view) {
    if (view == this) {
        return;
    }
    if (view->getId() == View::NO_ID) {
        LOGE("ConstraintHelper: views added to a helper need an id");
        return;
    }
    if (view->getParent() == nullptr) {
        LOGE("ConstraintHelper: views added to a helper need a parent");
        return;
    }
    mReferenceIds.clear();
    addRscID(view->getId());
    requestLayout();
}

int ConstraintHelper::removeView(View* view) {
    int index = -1;
    int id = view->getId();
    if (id == View::NO_ID) {
        return index;
    }
    mReferenceIds.clear();
    for (size_t i = 0; i < mIds.size(); i++) {
        if (mIds[i] == id) {
            index = (int) i;
            mIds.erase(mIds.begin() + i);
            break;
        }
    }
    requestLayout();
    return index;
}

bool ConstraintHelper::containsId(int id) const {
    for (int i : mIds) {
        if (i == id) return true;
    }
    return false;
}

int ConstraintHelper::indexFromId(int id) const {
    for (size_t i = 0; i < mIds.size(); i++) {
        if (mIds[i] == id) return (int) i;
    }
    return -1;
}

void ConstraintHelper::setUseViewMeasure(bool useViewMeasure) {
    mUseViewMeasure = useViewMeasure;
}

void ConstraintHelper::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    if (mUseViewMeasure) {
        View::onMeasure(widthMeasureSpec, heightMeasureSpec);
    } else {
        setMeasuredDimension(0, 0);
    }
}

void ConstraintHelper::validateParams() {
    // CDROID routes helper children to their core widget via ConstraintLayout::getViewWidget(),
    // so the LayoutParams swap done in the Java original is unnecessary here.
}

void ConstraintHelper::resolveRtl(ConstraintWidget* /*widget*/, bool /*isRtl*/) {
    // nothing (RTL deferred)
}

void ConstraintHelper::updatePreLayout(ConstraintLayout* container) {
    if (mHelperWidget == nullptr) {
        return;
    }
    mHelperWidget->removeAllIds();
    for (int id : mIds) {
        View* view = container->findViewById(id);
        if (view != nullptr) {
            ConstraintWidget* widget = container->getViewWidget(view);
            if (widget != nullptr) {
                mHelperWidget->add(widget);
            }
        }
    }
    mHelperWidget->updateConstraints(&container->getLayoutWidget());
}

void ConstraintHelper::updatePostLayout(ConstraintLayout* /*container*/) {}
void ConstraintHelper::updatePostMeasure(ConstraintLayout* /*container*/) {}
void ConstraintHelper::updatePreDraw(ConstraintLayout* /*container*/) {}

void ConstraintHelper::applyLayoutFeatures() {
    ViewGroup* parent = dynamic_cast<ViewGroup*>(getParent());
    if (parent != nullptr) {
        applyLayoutFeatures(dynamic_cast<ConstraintLayout*>(parent));
    }
}

void ConstraintHelper::applyLayoutFeatures(ConstraintLayout* container) {
    if (container == nullptr) {
        return;
    }
    int visibility = getVisibility();
    for (int id : mIds) {
        View* view = container->findViewById(id);
        if (view != nullptr) {
            view->setVisibility(visibility);
            // TODO: elevation/translationZ propagation (CDROID View lacks setElevation).
        }
    }
}

} // namespace cdroid
