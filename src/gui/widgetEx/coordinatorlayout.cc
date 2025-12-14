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
#include <widgetEx/coordinatorlayout.h>
#include <widgetEx/viewgrouputils.h>
#include <porting/cdlog.h>

namespace cdroid{

DECLARE_WIDGET(CoordinatorLayout)

CoordinatorLayout::CoordinatorLayout(int w, int h) :ViewGroup(w, h) {
    initView();
}

CoordinatorLayout::CoordinatorLayout(Context* context,const AttributeSet& attrs)
    :ViewGroup(context, attrs){
    initView();
    /*final TypedArray a = (defStyleAttr == 0)
            ? context.obtainStyledAttributes(attrs, R.styleable.CoordinatorLayout,
                0, R.style.Widget_Support_CoordinatorLayout)
            : context.obtainStyledAttributes(attrs, R.styleable.CoordinatorLayout,
                defStyleAttr, 0);*/
    std::string keylineArrayRes = attrs.getString("keylines");
    if (!keylineArrayRes.empty()) {
        context->getArray(keylineArrayRes,mKeylines);
        const float density = context->getDisplayMetrics().density;
        const size_t count = mKeylines.size();
        for (size_t i = 0; i < count; i++) {
            mKeylines[i] = (int) (mKeylines[i] * density);
        }
    }
    mStatusBarBackground = attrs.getDrawable("statusBarBackground");

}

void CoordinatorLayout::initView() {
    mDrawStatusBarBackground = false;
    mBehaviorTouchView = nullptr;
    mNestedScrollingTarget = nullptr;
    mStatusBarBackground = nullptr;
    mNestedScrollingParentHelper = new NestedScrollingParentHelper(this);
    setupForInsets();
    OnHierarchyChangeListener hcl;
    hcl.onChildViewAdded = [this](View&parent,View*child) {
            if (mOnHierarchyChangeListener.onChildViewAdded) {
                mOnHierarchyChangeListener.onChildViewAdded(parent, child);
            }
        };
    hcl.onChildViewRemoved = [this](View& parent, View* child) {
            onChildViewsChanged(EVENT_VIEW_REMOVED);
            if (mOnHierarchyChangeListener.onChildViewRemoved) {
                mOnHierarchyChangeListener.onChildViewRemoved(parent, child);
            }
        };
    ViewGroup::setOnHierarchyChangeListener(hcl);
}

CoordinatorLayout::~CoordinatorLayout() {
    delete mStatusBarBackground;
    delete mNestedScrollingParentHelper;
}

void CoordinatorLayout::setOnHierarchyChangeListener(const OnHierarchyChangeListener& onHierarchyChangeListener) {
    mOnHierarchyChangeListener = onHierarchyChangeListener;
}

void CoordinatorLayout::onAttachedToWindow() {
    ViewGroup::onAttachedToWindow();
    resetTouchBehaviors(false);
    if (mNeedsPreDrawListener) {
        if (mOnPreDrawListener == nullptr) {
            mOnPreDrawListener = [this]() {
                onChildViewsChanged(EVENT_PRE_DRAW);
                return true;
            };
        }
        ViewTreeObserver* vto = getViewTreeObserver();
        vto->addOnPreDrawListener(mOnPreDrawListener);
    }
    if (mLastInsets == nullptr && this->getFitsSystemWindows()) {
        // We're set to fitSystemWindows but we haven't had any insets yet...
        // We should request a new dispatch of window insets
        this->requestApplyInsets();
    }
    mIsAttachedToWindow = true;
}

void CoordinatorLayout::onDetachedFromWindow() {
    ViewGroup::onDetachedFromWindow();
    resetTouchBehaviors(false);
    if (mNeedsPreDrawListener && mOnPreDrawListener != nullptr) {
        ViewTreeObserver* vto = getViewTreeObserver();
        vto->removeOnPreDrawListener(mOnPreDrawListener);
    }
    if (mNestedScrollingTarget != nullptr) {
        onStopNestedScroll(mNestedScrollingTarget);
    }
    mIsAttachedToWindow = false;
}

void CoordinatorLayout::setStatusBarBackground(Drawable* bg) {
    if (mStatusBarBackground != bg) {
        if (mStatusBarBackground) {
            mStatusBarBackground->setCallback(nullptr);
        }
        mStatusBarBackground = bg ? bg->mutate() : nullptr;
        if (mStatusBarBackground) {
            if (mStatusBarBackground->isStateful()) {
                mStatusBarBackground->setState(getDrawableState());
            }
            mStatusBarBackground->setLayoutDirection(getLayoutDirection());
            mStatusBarBackground->setVisible(getVisibility() == VISIBLE, false);
            mStatusBarBackground->setCallback(this);
        }
        postInvalidateOnAnimation();
    }
}

Drawable* CoordinatorLayout::getStatusBarBackground()const {
    return mStatusBarBackground;
}

void CoordinatorLayout::drawableStateChanged() {
    ViewGroup::drawableStateChanged();

    bool changed = false;
    std::vector<int> state = getDrawableState();

    if (mStatusBarBackground != nullptr && mStatusBarBackground->isStateful()) {
        changed |= mStatusBarBackground->setState(state);
    }

    if (changed) {
        invalidate();
    }
}

 bool CoordinatorLayout::verifyDrawable(Drawable* who)const {
    return ViewGroup::verifyDrawable(who) || (who == mStatusBarBackground);
}

void CoordinatorLayout::setVisibility(int visibility) {
    ViewGroup::setVisibility(visibility);

    const bool visible = (visibility == View::VISIBLE);
    if (mStatusBarBackground && mStatusBarBackground->isVisible() != visible) {
        mStatusBarBackground->setVisible(visible, false);
    }
}

void CoordinatorLayout::setStatusBarBackgroundResource(const std::string& resId) {
    setStatusBarBackground(!resId.empty() ? mContext->getDrawable(resId) : nullptr);
}

void CoordinatorLayout::setStatusBarBackgroundColor(int color) {
    setStatusBarBackground(new ColorDrawable(color));
}

WindowInsets CoordinatorLayout::setWindowInsets(const WindowInsets& insets) {
    /*if ((mLastInsets == nullptr) || (*mLastInsets != insets)) {
        mLastInsets = insets;
        mDrawStatusBarBackground = insets != null && insets.getSystemWindowInsetTop() > 0;
        setWillNotDraw(!mDrawStatusBarBackground && getBackground() == null);

        // Now dispatch to the Behaviors
        insets = dispatchApplyWindowInsetsToBehaviors(insets);
        requestLayout();
    }*/
    return insets;
}

WindowInsets CoordinatorLayout::getLastWindowInsets() {
    return *mLastInsets;
} 

void CoordinatorLayout::resetTouchBehaviors(bool notifyOnInterceptTouchEvent) {
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        Behavior* b = lp->getBehavior();
        if (b != nullptr) {
            auto now = SystemClock::uptimeMillis();
            MotionEvent* cancelEvent = MotionEvent::obtain(now, now,MotionEvent::ACTION_CANCEL, 0.0f, 0.0f, 0);
            if (notifyOnInterceptTouchEvent) {
                b->onInterceptTouchEvent(*this, *child, *cancelEvent);
            } else {
                b->onTouchEvent(*this, *child, *cancelEvent);
            }
            cancelEvent->recycle();
        }
    }

    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        lp->resetTouchBehaviorTracking();
    }
    mBehaviorTouchView = nullptr;
    mDisallowInterceptReset = false;
}

static int TOP_SORTED_CHILDREN_COMPARATOR(View* lhs, View* rhs) {
    const float lz = lhs->getZ();
    const float rz = rhs->getZ();
    if (lz > rz) {
        return -1;
    } else if (lz < rz) {
        return 1;
    }
    return 0;
};

void CoordinatorLayout::getTopSortedChildren(std::vector<View*>& out) {
    out.clear();

    const bool useCustomOrder = isChildrenDrawingOrderEnabled();
    const int childCount = getChildCount();
    for (int i = childCount - 1; i >= 0; i--) {
        int childIndex = useCustomOrder ? getChildDrawingOrder(childCount, i) : i;
        View* child = getChildAt(childIndex);
        out.push_back(child);
    }
    std::sort(out.begin(), out.end(), TOP_SORTED_CHILDREN_COMPARATOR);
}

bool CoordinatorLayout::performIntercept(MotionEvent& ev,int type) {
    bool intercepted = false;
    bool newBlock = false;
    MotionEvent* cancelEvent = nullptr;
    const int action = ev.getActionMasked();

    std::vector<View*> topmostChildList;
    getTopSortedChildren(topmostChildList);

    // Let topmost child views inspect first
    const size_t childCount = topmostChildList.size();
    for (int i = 0; i < childCount; i++) {
        View* child = topmostChildList.at(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        Behavior* b = lp->getBehavior();

        if ((intercepted || newBlock) && (action != MotionEvent::ACTION_DOWN)){
            // Cancel all behaviors beneath the one that intercepted.
            // If the event is "down" then we don't have anything to cancel yet.
            if (b != nullptr) {
                if (cancelEvent == nullptr) {
                    const auto now = SystemClock::uptimeMillis();
                    cancelEvent = MotionEvent::obtain(now, now,MotionEvent::ACTION_CANCEL, 0.0f, 0.0f, 0);
                }
                switch (type) {
                case TYPE_ON_INTERCEPT:
                    b->onInterceptTouchEvent(*this, *child, *cancelEvent);
                    break;
                case TYPE_ON_TOUCH:
                    b->onTouchEvent(*this, *child, *cancelEvent);
                    break;
                }
            }
            continue;
        }

        if (!intercepted && (b != nullptr)) {
            switch (type) {
            case TYPE_ON_INTERCEPT:
                intercepted = b->onInterceptTouchEvent(*this, *child, ev);
                break;
            case TYPE_ON_TOUCH:
                intercepted = b->onTouchEvent(*this, *child, ev);
                break;
            }
            if (intercepted) {
                mBehaviorTouchView = child;
            }
        }

        // Don't keep going if we're not allowing interaction below this.
        // Setting newBlock will make sure we cancel the rest of the behaviors.
        const bool wasBlocking = lp->didBlockInteraction();
        const bool isBlocking = lp->isBlockingInteractionBelow(*this, child);
        newBlock = isBlocking && !wasBlocking;
        if (isBlocking && !newBlock) {
            // Stop here since we don't have anything more to cancel - we already did
            // when the behavior first started blocking things below this point.
            break;
        }
    }

    topmostChildList.clear();

    return intercepted;
}

bool CoordinatorLayout::onInterceptTouchEvent(MotionEvent& ev) {
    const int action = ev.getActionMasked();

    // Make sure we reset in case we had missed a previous important event.
    if (action == MotionEvent::ACTION_DOWN) {
        resetTouchBehaviors(true);
    }

    const bool intercepted = performIntercept(ev, TYPE_ON_INTERCEPT);

    if (action == MotionEvent::ACTION_UP || action == MotionEvent::ACTION_CANCEL) {
        resetTouchBehaviors(true);
    }

    return intercepted;
}

bool CoordinatorLayout::onTouchEvent(MotionEvent& ev) {
    bool handled = false;
    bool cancelSuper = false;
    MotionEvent* cancelEvent = nullptr;
    const int action = ev.getActionMasked();

    if (mBehaviorTouchView || (cancelSuper = performIntercept(ev, TYPE_ON_TOUCH))) {
        // Safe since performIntercept guarantees that
        // mBehaviorTouchView != null if it returns true
        LayoutParams* lp = (LayoutParams*) mBehaviorTouchView->getLayoutParams();
        Behavior* b = lp->getBehavior();
        if (b != nullptr) {
            handled = b->onTouchEvent(*this, *mBehaviorTouchView, ev);
        }
    }

    // Keep the super implementation correct
    if (mBehaviorTouchView == nullptr) {
        handled |= ViewGroup::onTouchEvent(ev);
    } else if (cancelSuper) {
        if (cancelEvent == nullptr) {
            const auto now = SystemClock::uptimeMillis();
            cancelEvent = MotionEvent::obtain(now, now, MotionEvent::ACTION_CANCEL, 0.0f, 0.0f, 0);
        }
        ViewGroup::onTouchEvent(*cancelEvent);
    }

    if (!handled && action == MotionEvent::ACTION_DOWN) {

    }

    if (cancelEvent != nullptr) {
        cancelEvent->recycle();
    }

    if (action == MotionEvent::ACTION_UP || action == MotionEvent::ACTION_CANCEL) {
        resetTouchBehaviors(false);
    }

    return handled;
}

void CoordinatorLayout::requestDisallowInterceptTouchEvent(bool disallowIntercept) {
    ViewGroup::requestDisallowInterceptTouchEvent(disallowIntercept);
    if (disallowIntercept && !mDisallowInterceptReset) {
        resetTouchBehaviors(false);
        mDisallowInterceptReset = true;
    }
}

int CoordinatorLayout::getKeyline(int index) const{
    if (mKeylines.empty()){//} == nullptr) {
        LOGE("No keylines defined for %p attempted index lookup %d" ,this,index);
        return 0;
    }

    if (index < 0 || index >= mKeylines.size()) {
        LOGE("%p Keyline index %d out of range for ",this,index);
        return 0;
    }

    return mKeylines[index];
}

CoordinatorLayout::Behavior* CoordinatorLayout::parseBehavior(Context* context,const AttributeSet& attrs,const std::string& name) {
    if (name.empty()){//TextUtils.isEmpty(name)) {
        return nullptr;
    }
#if 0
    std::string fullName;
    if (name.startsWith(".")) {
        // Relative to the app package. Prepend the app package name.
        fullName = context.getPackageName() + name;
    } else if (name.indexOf('.') >= 0) {
        // Fully qualified package name.
        fullName = name;
    } else {
        // Assume stock behavior in this package (if we have one)
        fullName = !TextUtils.isEmpty(WIDGET_PACKAGE_NAME)
                ? (WIDGET_PACKAGE_NAME + '.' + name)
                : name;
    }

    try {
        Map<String, Constructor<Behavior>> constructors = sConstructors.get();
        if (constructors == null) {
            constructors = new HashMap<>();
            sConstructors.set(constructors);
        }
        Constructor<Behavior> c = constructors.get(fullName);
        if (c == null) {
            final Class<Behavior> clazz = (Class<Behavior>) context.getClassLoader()
                    .loadClass(fullName);
            c = clazz.getConstructor(CONSTRUCTOR_PARAMS);
            c.setAccessible(true);
            constructors.put(fullName, c);
        }
        return c.newInstance(context, attrs);
    } catch (Exception e) {
        throw std::runtime_error("Could not inflate Behavior subclass " + fullName, e);
    }
#endif
    return nullptr;
}

CoordinatorLayout::LayoutParams* CoordinatorLayout::getResolvedLayoutParams(View* child) {
    LayoutParams* result = (LayoutParams*) child->getLayoutParams();
    if (!result->mBehaviorResolved) {
        if (dynamic_cast<AttachedBehavior*>(child)) {
            Behavior* attachedBehavior = ((AttachedBehavior*) child)->getBehavior();
            if (attachedBehavior == nullptr) {
                LOGE("Attached behavior class is null");
            }
            result->setBehavior(attachedBehavior);
            result->mBehaviorResolved = true;
        } else {
            // The deprecated path that looks up the attached behavior based on annotation
            /*Class< ? > childClass = child.getClass();
            Behavior* defaultBehavior = nullptr;
            while (childClass != null
                    && (defaultBehavior = childClass.getAnnotation(DefaultBehavior.class))
                            == null) {
                childClass = childClass.getSuperclass();
            }
            if (defaultBehavior != null) {
                try {
                    result->setBehavior(
                            defaultBehavior.value().getDeclaredConstructor().newInstance());
                } catch (Exception e) {
                    LOGE("Default behavior class " + defaultBehavior.value().getName()
                                    + " could not be instantiated. Did you forget"
                                    + " a default constructor?", e);
                }
            }*/
            result->mBehaviorResolved = true;
        }
    }
    return result;
}

void CoordinatorLayout::prepareChildren() {
    mDependencySortedChildren.clear();
    mChildDag.clear();

    for (int i = 0, count = getChildCount(); i < count; i++) {
        View* view = getChildAt(i);
        LayoutParams* lp = getResolvedLayoutParams(view);
        lp->findAnchorView(*this, view);

        mChildDag.addNode(view);

        // Now iterate again over the other children, adding any dependencies to the graph
        for (int j = 0; j < count; j++) {
            if (j == i) {
                continue;
            }
            View* other = getChildAt(j);
            if (lp->dependsOn(*this, view, other)) {
                if (!mChildDag.contains(other)) {
                    // Make sure that the other node is added
                    mChildDag.addNode(other);
                }
                // Now add the dependency to the graph
                mChildDag.addEdge(other, view);
            }
        }
    }

    // Finally add the sorted graph list to our list
    auto sl = mChildDag.getSortedList();
    mDependencySortedChildren.insert(mDependencySortedChildren.end(),sl.begin(),sl.end());// addAll();
    // We also need to reverse the result since we want the start of the list to contain
    // Views which have no dependencies, then dependent views after that
    //Collections.reverse(mDependencySortedChildren);
}

void CoordinatorLayout::getDescendantRect(View* descendant, Rect& out) {
    ViewGroupUtils::getDescendantRect(this, descendant, out);
}

int CoordinatorLayout::getSuggestedMinimumWidth() {
    return std::max(ViewGroup::getSuggestedMinimumWidth(), getPaddingLeft() + getPaddingRight());
}

int CoordinatorLayout::getSuggestedMinimumHeight() {
    return std::max(ViewGroup::getSuggestedMinimumHeight(), getPaddingTop() + getPaddingBottom());
}

void CoordinatorLayout::onMeasureChild(View* child, int parentWidthMeasureSpec, int widthUsed,
        int parentHeightMeasureSpec, int heightUsed) {
    measureChildWithMargins(child, parentWidthMeasureSpec, widthUsed,parentHeightMeasureSpec, heightUsed);
}

void CoordinatorLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    prepareChildren();
    ensurePreDrawListener();

    const int paddingLeft = getPaddingLeft();
    const int paddingTop = getPaddingTop();
    const int paddingRight = getPaddingRight();
    const int paddingBottom = getPaddingBottom();
    const int layoutDirection = this->getLayoutDirection();
    const bool isRtl = layoutDirection == View::LAYOUT_DIRECTION_RTL;
    const int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    const int widthSize = MeasureSpec::getSize(widthMeasureSpec);
    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    const int heightSize = MeasureSpec::getSize(heightMeasureSpec);

    const int widthPadding = paddingLeft + paddingRight;
    const int heightPadding = paddingTop + paddingBottom;
    int widthUsed = getSuggestedMinimumWidth();
    int heightUsed = getSuggestedMinimumHeight();
    int childState = 0;

    const bool applyInsets = 0;//mLastInsets && this->getFitsSystemWindows();

    const size_t childCount = mDependencySortedChildren.size();
    for (size_t i = 0; i < childCount; i++) {
        View* child = mDependencySortedChildren.at(i);
        if (child->getVisibility() == GONE) {
            // If the child is GONE, skip...
            continue;
        }

        const LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

        int keylineWidthUsed = 0;
        if (lp->keyline >= 0 && widthMode != MeasureSpec::UNSPECIFIED) {
            const int keylinePos = getKeyline(lp->keyline);
            const int keylineGravity = Gravity::getAbsoluteGravity(
                    resolveKeylineGravity(lp->gravity), layoutDirection)
                    & Gravity::HORIZONTAL_GRAVITY_MASK;
            if ((keylineGravity == Gravity::LEFT && !isRtl)
                    || (keylineGravity == Gravity::RIGHT && isRtl)) {
                keylineWidthUsed = std::max(0, widthSize - paddingRight - keylinePos);
            } else if ((keylineGravity == Gravity::RIGHT && !isRtl)
                    || (keylineGravity == Gravity::LEFT && isRtl)) {
                keylineWidthUsed = std::max(0, keylinePos - paddingLeft);
            }
        }

        int childWidthMeasureSpec = widthMeasureSpec;
        int childHeightMeasureSpec = heightMeasureSpec;
        if (applyInsets && !child->getFitsSystemWindows()) {
            // We're set to handle insets but this child isn't, so we will measure the
            // child as if there are no insets
            const int horizInsets = mLastInsets->getSystemWindowInsetLeft()
                    + mLastInsets->getSystemWindowInsetRight();
            const int vertInsets = mLastInsets->getSystemWindowInsetTop()
                    + mLastInsets->getSystemWindowInsetBottom();

            childWidthMeasureSpec = MeasureSpec::makeMeasureSpec(widthSize - horizInsets, widthMode);
            childHeightMeasureSpec = MeasureSpec::makeMeasureSpec(heightSize - vertInsets, heightMode);
        }

        Behavior* b = lp->getBehavior();
        if ((b == nullptr) || !b->onMeasureChild(*this, *child, childWidthMeasureSpec, keylineWidthUsed,
                childHeightMeasureSpec, 0)) {
            onMeasureChild(child, childWidthMeasureSpec, keylineWidthUsed, childHeightMeasureSpec, 0);
        }

        widthUsed = std::max(widthUsed, widthPadding + child->getMeasuredWidth() +
                lp->leftMargin + lp->rightMargin);

        heightUsed = std::max(heightUsed, heightPadding + child->getMeasuredHeight() +
                lp->topMargin + lp->bottomMargin);
        childState = View::combineMeasuredStates(childState, child->getMeasuredState());
    }

    const int width = View::resolveSizeAndState(widthUsed, widthMeasureSpec, childState & View::MEASURED_STATE_MASK);
    const int height = View::resolveSizeAndState(heightUsed, heightMeasureSpec, childState << View::MEASURED_HEIGHT_STATE_SHIFT);
    setMeasuredDimension(width, height);
}

WindowInsets CoordinatorLayout::dispatchApplyWindowInsetsToBehaviors(WindowInsets insets) {
    if (insets.isConsumed()) {
        return insets;
    }

    for (int i = 0, z = getChildCount(); i < z; i++) {
        View* child = getChildAt(i);
        if (child->getFitsSystemWindows()) {
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            Behavior* b = lp->getBehavior();

            if (b != nullptr) {
                // If the view has a behavior, let it try first
                insets = b->onApplyWindowInsets(*this, *child, insets);
                if (insets.isConsumed()) {
                    // If it consumed the insets, break
                    break;
                }
            }
        }
    }

    return insets;
}


void CoordinatorLayout::onLayoutChild(View* child, int layoutDirection) {
    const LayoutParams* lp = (const LayoutParams*) child->getLayoutParams();
    if (lp->checkAnchorChanged()) {
        LOGE("An anchor may not be changed after CoordinatorLayout measurement begins before layout is complete.");
    }
    if (lp->mAnchorView != nullptr) {
        layoutChildWithAnchor(child, lp->mAnchorView, layoutDirection);
    } else if (lp->keyline >= 0) {
        layoutChildWithKeyline(child, lp->keyline, layoutDirection);
    } else {
        layoutChild(child, layoutDirection);
    }
}

void CoordinatorLayout::onLayout(bool changed, int l, int t, int r, int b) {
    const int layoutDirection = getLayoutDirection();
    const size_t childCount = mDependencySortedChildren.size();
    for (size_t i = 0; i < childCount; i++) {
        View* child = mDependencySortedChildren.at(i);
        if (child->getVisibility() == View::GONE) {
            // If the child is GONE, skip...
            continue;
        }

        const LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        Behavior* behavior = lp->getBehavior();

        if ((behavior == nullptr) || !behavior->onLayoutChild(*this, *child, layoutDirection)) {
            onLayoutChild(child, layoutDirection);
        }
    }
}

void CoordinatorLayout::onDraw(Canvas& c) {
    ViewGroup::onDraw(c);
    if (mDrawStatusBarBackground && mStatusBarBackground) {
        int inset =  mLastInsets ? mLastInsets->getSystemWindowInsetTop() : 0;
        if (inset > 0) {
            mStatusBarBackground->setBounds(0, 0, getWidth(), inset);
            mStatusBarBackground->draw(c);
        }
    }
}

void CoordinatorLayout::setFitsSystemWindows(bool fitSystemWindows) {
    ViewGroup::setFitsSystemWindows(fitSystemWindows);
    setupForInsets();
}


void CoordinatorLayout::recordLastChildRect(View* child, Rect& r) {
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    lp->setLastChildRect(r);
}


void CoordinatorLayout::getLastChildRect(View* child, Rect& out) {
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    out = lp->getLastChildRect();
}


void CoordinatorLayout::getChildRect(View* child, bool transform, Rect& out) {
    if (child->isLayoutRequested() || (child->getVisibility() == View::GONE)) {
        out.setEmpty();
        return;
    }
    if (transform) {
        getDescendantRect(child, out);
    } else {
        out.set(child->getLeft(), child->getTop(), child->getWidth(), child->getHeight());
    }
}

void CoordinatorLayout::getDesiredAnchoredChildRectWithoutConstraints(View* child, int layoutDirection,
        const Rect& anchorRect, Rect& out,const LayoutParams* lp, int childWidth, int childHeight) {
    const int absGravity = Gravity::getAbsoluteGravity(
            resolveAnchoredChildGravity(lp->gravity), layoutDirection);
    const int absAnchorGravity = Gravity::getAbsoluteGravity(
            resolveGravity(lp->anchorGravity), layoutDirection);

    const int hgrav = absGravity & Gravity::HORIZONTAL_GRAVITY_MASK;
    const int vgrav = absGravity & Gravity::VERTICAL_GRAVITY_MASK;
    const int anchorHgrav = absAnchorGravity & Gravity::HORIZONTAL_GRAVITY_MASK;
    const int anchorVgrav = absAnchorGravity & Gravity::VERTICAL_GRAVITY_MASK;

    int left;
    int top;

    // Align to the anchor. This puts us in an assumed right/bottom child view gravity.
    // If this is not the case we will subtract out the appropriate portion of
    // the child size below.
    switch (anchorHgrav) {
    default:
    case Gravity::LEFT:
        left = anchorRect.left;
        break;
    case Gravity::RIGHT:
        left = anchorRect.right();
        break;
    case Gravity::CENTER_HORIZONTAL:
        left = anchorRect.left + anchorRect.width / 2;
        break;
    }

    switch (anchorVgrav) {
    default:
    case Gravity::TOP:
        top = anchorRect.top;
        break;
    case Gravity::BOTTOM:
        top = anchorRect.bottom();
        break;
    case Gravity::CENTER_VERTICAL:
        top = anchorRect.top + anchorRect.height / 2;
        break;
    }

    // Offset by the child view's gravity itself. The above assumed right/bottom gravity.
    switch (hgrav) {
    default:
    case Gravity::LEFT:
        left -= childWidth;
        break;
    case Gravity::RIGHT:
        // Do nothing, we're already in position.
        break;
    case Gravity::CENTER_HORIZONTAL:
        left -= childWidth / 2;
        break;
    }

    switch (vgrav) {
    default:
    case Gravity::TOP:
        top -= childHeight;
        break;
    case Gravity::BOTTOM:
        // Do nothing, we're already in position.
        break;
    case Gravity::CENTER_VERTICAL:
        top -= childHeight / 2;
        break;
    }

    out.set(left, top, left + childWidth, top + childHeight);
}

void CoordinatorLayout::constrainChildRect(const LayoutParams& lp, Rect& out, int childWidth, int childHeight) {
    const int width = getWidth();
    const int height = getHeight();

    // Obey margins and padding
    int left = std::max(getPaddingLeft() + lp.leftMargin,
            std::min(out.left,width - getPaddingRight() - childWidth - lp.rightMargin));
    int top = std::max(getPaddingTop() + lp.topMargin,
            std::min(out.top,height - getPaddingBottom() - childHeight - lp.bottomMargin));

    out.set(left, top, childWidth, childHeight);
}


void CoordinatorLayout::getDesiredAnchoredChildRect(View* child, int layoutDirection,const Rect& anchorRect, Rect& out) {
    const LayoutParams* lp = (const LayoutParams*) child->getLayoutParams();
    const int childWidth = child->getMeasuredWidth();
    const int childHeight = child->getMeasuredHeight();
    getDesiredAnchoredChildRectWithoutConstraints(child, layoutDirection, anchorRect, out, lp,
            childWidth, childHeight);
    constrainChildRect(*lp, out, childWidth, childHeight);
}

void CoordinatorLayout::layoutChildWithAnchor(View* child, View* anchor, int layoutDirection) {
    Rect anchorRect,childRect;
    getDescendantRect(anchor, anchorRect);
    getDesiredAnchoredChildRect(child, layoutDirection, anchorRect, childRect);
    child->layout(childRect.left, childRect.top, childRect.width, childRect.height);
}

void CoordinatorLayout::layoutChildWithKeyline(View* child, int keyline, int layoutDirection) {
    const LayoutParams* lp = (const LayoutParams*) child->getLayoutParams();
    const int absGravity = Gravity::getAbsoluteGravity(resolveKeylineGravity(lp->gravity), layoutDirection);

    const int hgrav = absGravity & Gravity::HORIZONTAL_GRAVITY_MASK;
    const int vgrav = absGravity & Gravity::VERTICAL_GRAVITY_MASK;
    const int width = getWidth();
    const int height = getHeight();
    const int childWidth = child->getMeasuredWidth();
    const int childHeight = child->getMeasuredHeight();

    if (layoutDirection == View::LAYOUT_DIRECTION_RTL) {
        keyline = width - keyline;
    }

    int left = getKeyline(keyline) - childWidth;
    int top = 0;

    switch (hgrav) {
    default:
    case Gravity::LEFT:
        // Nothing to do.
        break;
    case Gravity::RIGHT:
        left += childWidth;
        break;
    case Gravity::CENTER_HORIZONTAL:
        left += childWidth / 2;
        break;
    }

    switch (vgrav) {
    default:
    case Gravity::TOP:
        // Do nothing, we're already in position.
        break;
    case Gravity::BOTTOM:
        top += childHeight;
        break;
    case Gravity::CENTER_VERTICAL:
        top += childHeight / 2;
        break;
    }

    // Obey margins and padding
    left = std::max(getPaddingLeft() + lp->leftMargin,
            std::min(left,width - getPaddingRight() - childWidth - lp->rightMargin));
    top = std::max(getPaddingTop() + lp->topMargin,
            std::min(top,height - getPaddingBottom() - childHeight - lp->bottomMargin));

    child->layout(left, top,childWidth,childHeight);
}

void CoordinatorLayout::layoutChild(View* child, int layoutDirection) {
    const LayoutParams* lp = (const LayoutParams*) child->getLayoutParams();
    Rect parent,out;
    parent.set(getPaddingLeft() + lp->leftMargin,
            getPaddingTop() + lp->topMargin,
            getWidth() - getPaddingRight() - lp->rightMargin,
            getHeight() - getPaddingBottom() - lp->bottomMargin);

    if (mLastInsets && getFitsSystemWindows() && !child->getFitsSystemWindows()) {
        // If we're set to handle insets but this child isn't, then it has been measured as
        // if there are no insets. We need to lay it out to match.
        parent.left += mLastInsets->getSystemWindowInsetLeft();
        parent.top += mLastInsets->getSystemWindowInsetTop();
        parent.width -= mLastInsets->getSystemWindowInsetRight();
        parent.height -= mLastInsets->getSystemWindowInsetBottom();
    }

    Gravity::apply(resolveGravity(lp->gravity), child->getMeasuredWidth(),
            child->getMeasuredHeight(), parent, out, layoutDirection);
    child->layout(out.left, out.top, out.width, out.height);
}

int CoordinatorLayout::resolveGravity(int gravity) {
    if ((gravity & Gravity::HORIZONTAL_GRAVITY_MASK) == Gravity::NO_GRAVITY) {
        gravity |= Gravity::START;
    }
    if ((gravity & Gravity::VERTICAL_GRAVITY_MASK) == Gravity::NO_GRAVITY) {
        gravity |= Gravity::TOP;
    }
    return gravity;
}

int CoordinatorLayout::resolveKeylineGravity(int gravity) {
    return gravity == Gravity::NO_GRAVITY ? Gravity::END | Gravity::TOP : gravity;
}

int CoordinatorLayout::resolveAnchoredChildGravity(int gravity) {
    return gravity == Gravity::NO_GRAVITY ? Gravity::CENTER : gravity;
}

bool CoordinatorLayout::drawChild(Canvas& canvas, View* child, int64_t drawingTime) {
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    if (lp->mBehavior != nullptr) {
        float scrimAlpha = lp->mBehavior->getScrimOpacity(*this, *child);
        /*if (scrimAlpha > .0f) {
            if (mScrimPaint == null) {
                mScrimPaint = new Paint();
            }
            mScrimPaint.setColor(lp->mBehavior->getScrimColor(*this, *child));
            mScrimPaint.setAlpha(clamp(std::round(255 * scrimAlpha), 0, 255));

            canvas.save();
            if (child->isOpaque()) {
                // If the child is opaque, there is no need to draw behind it so we'll inverse
                // clip the canvas
                canvas.clipRect(child.getLeft(), child.getTop(), child.getRight(),
                        child.getBottom(), Region.Op.DIFFERENCE);
            }
            // Now draw the rectangle for the scrim
            canvas.drawRect(getPaddingLeft(), getPaddingTop(),
                    getWidth() - getPaddingRight(), getHeight() - getPaddingBottom(),
                    mScrimPaint);
            canvas.restore();
        }*/
    }
    return ViewGroup::drawChild(canvas, child, drawingTime);
}

static int clamp(int value, int min, int max) {
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    }
    return value;
}

void CoordinatorLayout::onChildViewsChanged(int type) {
    const int layoutDirection = this->getLayoutDirection();
    const size_t childCount = mDependencySortedChildren.size();
    Rect inset;
    Rect drawRect;
    Rect lastDrawRect;

    for (size_t i = 0; i < childCount; i++) {
        View* child = mDependencySortedChildren.at(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        if (type == EVENT_PRE_DRAW && child->getVisibility() == View::GONE) {
            // Do not try to update GONE child views in pre draw updates.
            continue;
        }

        // Check child views before for anchor
        for (size_t j = 0; j < i; j++) {
            View* checkChild = mDependencySortedChildren.at(j);

            if (lp->mAnchorDirectChild == checkChild) {
                offsetChildToAnchor(child, layoutDirection);
            }
        }

        // Get the current draw rect of the view
        getChildRect(child, true, drawRect);

        // Accumulate inset sizes
        if (lp->insetEdge != Gravity::NO_GRAVITY && !drawRect.empty()) {
            const int absInsetEdge = Gravity::getAbsoluteGravity(lp->insetEdge, layoutDirection);
            switch (absInsetEdge & Gravity::VERTICAL_GRAVITY_MASK) {
            case Gravity::TOP:
                inset.top = std::max(inset.top, drawRect.bottom());
                break;
            case Gravity::BOTTOM:
                inset.height = std::max(inset.height, getHeight() - drawRect.top);
                break;
            }
            switch (absInsetEdge & Gravity::HORIZONTAL_GRAVITY_MASK) {
            case Gravity::LEFT:
                inset.left = std::max(inset.left, drawRect.right());
                break;
            case Gravity::RIGHT:
                inset.width = std::max(inset.width, getWidth() - drawRect.left);
                break;
            }
        }

        // Dodge inset edges if necessary
        if (lp->dodgeInsetEdges != Gravity::NO_GRAVITY && child->getVisibility() == View::VISIBLE) {
            offsetChildByInset(child, inset, layoutDirection);
        }

        if (type != EVENT_VIEW_REMOVED) {
            // Did it change? if not continue
            getLastChildRect(child, lastDrawRect);
            if (lastDrawRect==drawRect) {
                continue;
            }
            recordLastChildRect(child, drawRect);
        }

        // Update any behavior-dependent views for the change
        for (size_t j = i + 1; j < childCount; j++) {
            View* checkChild = mDependencySortedChildren.at(j);
            LayoutParams* checkLp = (LayoutParams*) checkChild->getLayoutParams();
            Behavior* b = checkLp->getBehavior();

            if (b && b->layoutDependsOn(*this, *checkChild, *child)) {
                if (type == EVENT_PRE_DRAW && checkLp->getChangedAfterNestedScroll()) {
                    // If this is from a pre-draw and we have already been changed
                    // from a nested scroll, skip the dispatch and reset the flag
                    checkLp->resetChangedAfterNestedScroll();
                    continue;
                }

                bool handled;
                switch (type) {
                case EVENT_VIEW_REMOVED:
                    // EVENT_VIEW_REMOVED means that we need to dispatch
                    // onDependentViewRemoved() instead
                    b->onDependentViewRemoved(*this, *checkChild, *child);
                    handled = true;
                    break;
                default:
                    // Otherwise we dispatch onDependentViewChanged()
                    handled = b->onDependentViewChanged(*this, *checkChild, *child);
                    break;
                }

                if (type == EVENT_NESTED_SCROLL) {
                    // If this is from a nested scroll, set the flag so that we may skip
                    // any resulting onPreDraw dispatch (if needed)
                    checkLp->setChangedAfterNestedScroll(handled);
                }
            }
        }
    }
}

void CoordinatorLayout::offsetChildByInset(View* child,const Rect& inset,int layoutDirection) {
    if (!child->isLaidOut()) {
        // The view has not been laid out yet, so we can't obtain its bounds.
        return;
    }

    if (child->getWidth() <= 0 || child->getHeight() <= 0) {
        // Bounds are empty so there is nothing to dodge against, skip...
        return;
    }

    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    Behavior* behavior = lp->getBehavior();
    Rect dodgeRect, bounds;
    bounds.set(child->getLeft(), child->getTop(), child->getWidth(), child->getHeight());

    if (behavior && behavior->getInsetDodgeRect(*this, *child, dodgeRect)) {
        // Make sure that the rect is within the view's bounds
        if (!bounds.contains(dodgeRect)) {
            LOGE("Rect should be within the child's bounds.");
               //+ " Rect:" + dodgeRect.toShortString() " | Bounds:" + bounds.toShortString());
        }
    } else {
        dodgeRect = bounds;
    }


    if (dodgeRect.empty()) {
        // Rect is empty so there is nothing to dodge against, skip...
        return;
    }

    const int absDodgeInsetEdges = Gravity::getAbsoluteGravity(lp->dodgeInsetEdges,layoutDirection);

    bool offsetY = false;
    if ((absDodgeInsetEdges & Gravity::TOP) == Gravity::TOP) {
        const int distance = dodgeRect.top - lp->topMargin - lp->mInsetOffsetY;
        if (distance < inset.top) {
            setInsetOffsetY(child, inset.top - distance);
            offsetY = true;
        }
    }
    if ((absDodgeInsetEdges & Gravity::BOTTOM) == Gravity::BOTTOM) {
        const int distance = getHeight() - dodgeRect.bottom() - lp->bottomMargin + lp->mInsetOffsetY;
        if (distance < inset.bottom()) {
            setInsetOffsetY(child, distance - inset.bottom());
            offsetY = true;
        }
    }
    if (!offsetY) {
        setInsetOffsetY(child, 0);
    }

    bool offsetX = false;
    if ((absDodgeInsetEdges & Gravity::LEFT) == Gravity::LEFT) {
        const int distance = dodgeRect.left - lp->leftMargin - lp->mInsetOffsetX;
        if (distance < inset.left) {
            setInsetOffsetX(child, inset.left - distance);
            offsetX = true;
        }
    }
    if ((absDodgeInsetEdges & Gravity::RIGHT) == Gravity::RIGHT) {
        const int distance = getWidth() - dodgeRect.right() - lp->rightMargin + lp->mInsetOffsetX;
        if (distance < inset.right()) {
            setInsetOffsetX(child, distance - inset.right());
            offsetX = true;
        }
    }
    if (!offsetX) {
        setInsetOffsetX(child, 0);
    }
}

void CoordinatorLayout::setInsetOffsetX(View* child, int offsetX) {
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    if (lp->mInsetOffsetX != offsetX) {
        const int dx = offsetX - lp->mInsetOffsetX;
        child->offsetLeftAndRight(dx);
        lp->mInsetOffsetX = offsetX;
    }
}

void CoordinatorLayout::setInsetOffsetY(View* child, int offsetY) {
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    if (lp->mInsetOffsetY != offsetY) {
        const int dy = offsetY - lp->mInsetOffsetY;
        child->offsetTopAndBottom(dy);
        lp->mInsetOffsetY = offsetY;
    }
}

void CoordinatorLayout::dispatchDependentViewsChanged(View& view) {
    std::vector<View*>* dependents = mChildDag.getIncomingEdges(&view);
    if (dependents&&dependents->size()) {
        for (int i = 0; i < dependents->size(); i++) {
            View* child = dependents->at(i);
            LayoutParams* lp = (LayoutParams*)child->getLayoutParams();
            Behavior* b = lp->getBehavior();
            if (b != nullptr) {
                b->onDependentViewChanged(*this, *child, view);
            }
        }
    }
}

std::vector<View*> CoordinatorLayout::getDependencies(View& child) {
    std::vector<View*>* dependencies = mChildDag.getOutgoingEdges(&child);
    mTempDependenciesList.clear();
    if (dependencies&&dependencies->size()) {
        mTempDependenciesList.insert(mTempDependenciesList.end(),dependencies->begin(), dependencies->end());
    }
    return mTempDependenciesList;
}

std::vector<View*> CoordinatorLayout::getDependents(View& child) {
    std::vector<View*>* edges = mChildDag.getIncomingEdges(&child);
    mTempDependenciesList.clear();
    if (edges&&edges->size()) {
        mTempDependenciesList.insert(mTempDependenciesList.end(),edges->begin(), edges->end());
    }
    return mTempDependenciesList;
}

std::vector<View*> CoordinatorLayout::getDependencySortedChildren() {
    prepareChildren();
    return mDependencySortedChildren;// Collections.unmodifiableList(mDependencySortedChildren);
}

/**
 * Add or remove the pre-draw listener as necessary.
 */
void CoordinatorLayout::ensurePreDrawListener() {
    bool bHasDependencies = false;
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        if (hasDependencies(child)) {
            bHasDependencies = true;
            break;
        }
    }

    if (bHasDependencies != mNeedsPreDrawListener) {
        if (bHasDependencies) {
            addPreDrawListener();
        } else {
            removePreDrawListener();
        }
    }
}

bool CoordinatorLayout::hasDependencies(View* child)const {
    return mChildDag.hasOutgoingEdges(child);
}


void CoordinatorLayout::addPreDrawListener() {
    if (mIsAttachedToWindow) {
        // Add the listener
        if (mOnPreDrawListener == nullptr) {
            mOnPreDrawListener = [this]() {
                onChildViewsChanged(EVENT_PRE_DRAW);
                return true;
            };
        }
        ViewTreeObserver* vto = getViewTreeObserver();
        vto->addOnPreDrawListener(mOnPreDrawListener);
    }
    // Record that we need the listener regardless of whether or not we're attached.
    // We'll add the real listener when we become attached.
    mNeedsPreDrawListener = true;
}


void CoordinatorLayout::removePreDrawListener() {
    if (mIsAttachedToWindow) {
        if (mOnPreDrawListener != nullptr) {
            ViewTreeObserver* vto = getViewTreeObserver();
            vto->removeOnPreDrawListener(mOnPreDrawListener);
        }
    }
    mNeedsPreDrawListener = false;
}


void CoordinatorLayout::offsetChildToAnchor(View* child, int layoutDirection) {
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    if (lp->mAnchorView != nullptr) {
        Rect anchorRect;
        Rect childRect;
        Rect desiredChildRect;

        getDescendantRect(lp->mAnchorView, anchorRect);
        getChildRect(child, false, childRect);

        int childWidth = child->getMeasuredWidth();
        int childHeight = child->getMeasuredHeight();
        getDesiredAnchoredChildRectWithoutConstraints(child, layoutDirection, anchorRect,
                desiredChildRect, lp, childWidth, childHeight);
        const bool changed = (desiredChildRect.left != childRect.left) ||
                (desiredChildRect.top != childRect.top);
        constrainChildRect(*lp, desiredChildRect, childWidth, childHeight);

        const int dx = desiredChildRect.left - childRect.left;
        const int dy = desiredChildRect.top - childRect.top;

        if (dx != 0) {
            child->offsetLeftAndRight(dx);
        }
        if (dy != 0) {
            child->offsetTopAndBottom(dy);
        }

        if (changed) {
            // If we have needed to move, make sure to notify the child's Behavior
            Behavior* b = lp->getBehavior();
            if (b != nullptr) {
                b->onDependentViewChanged(*this, *child, *lp->mAnchorView);
            }
        }

    }
}

bool CoordinatorLayout::isPointInChildBounds(View& child, int x, int y) {
    Rect r;
    getDescendantRect(&child, r);
    return r.contains(x, y);
}

bool CoordinatorLayout::doViewsOverlap(View& first, View& second) {
    if (first.getVisibility() == VISIBLE && second.getVisibility() == VISIBLE) {
        Rect firstRect,secondRect;
        getChildRect(&first, first.getParent() != this, firstRect);
        getChildRect(&second, second.getParent() != this, secondRect);
        return !(firstRect.left > secondRect.right() || firstRect.top > secondRect.bottom()
                    || firstRect.right() < secondRect.left || firstRect.bottom() < secondRect.top);
    }
    return false;
}

CoordinatorLayout::LayoutParams* CoordinatorLayout::generateLayoutParams(const AttributeSet& attrs)const {
    return new LayoutParams(getContext(), attrs);
}

CoordinatorLayout::LayoutParams* CoordinatorLayout::generateLayoutParams(const ViewGroup::LayoutParams* p)const {
    if (dynamic_cast<const LayoutParams*>(p)) {
        return new LayoutParams((const LayoutParams&)*p);
    } else if (dynamic_cast<const MarginLayoutParams*>(p)) {
        return new LayoutParams((const MarginLayoutParams&)*p);
    }
    return new LayoutParams(*p);
}

CoordinatorLayout::LayoutParams* CoordinatorLayout::generateDefaultLayoutParams()const {
    return new LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
}

 bool CoordinatorLayout::checkLayoutParams(const ViewGroup::LayoutParams* p)const {
    return dynamic_cast<const LayoutParams*>(p) && ViewGroup::checkLayoutParams(p);
}

bool CoordinatorLayout::onStartNestedScroll(View* child, View* target, int nestedScrollAxes) {
    return onStartNestedScroll(child, target, nestedScrollAxes, View::TYPE_TOUCH);
}

bool CoordinatorLayout::onStartNestedScroll(View* child, View* target, int axes, int type) {
    bool handled = false;

    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* view = getChildAt(i);
        if (view->getVisibility() == View::GONE) {
            // If it's GONE, don't dispatch
            continue;
        }
        LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
        Behavior* viewBehavior = lp->getBehavior();
        if (viewBehavior != nullptr) {
            const bool accepted = viewBehavior->onStartNestedScroll(*this, *view, *child,
                    *target, axes, type);
            handled |= accepted;
            lp->setNestedScrollAccepted(type, accepted);
        } else {
            lp->setNestedScrollAccepted(type, false);
        }
    }
    return handled;
}

void CoordinatorLayout::onNestedScrollAccepted(View* child, View* target, int nestedScrollAxes) {
    onNestedScrollAccepted(child, target, nestedScrollAxes, View::TYPE_TOUCH);
}

void CoordinatorLayout::onNestedScrollAccepted(View* child, View* target, int nestedScrollAxes, int type) {
    mNestedScrollingParentHelper->onNestedScrollAccepted(child, target, nestedScrollAxes, type);
    mNestedScrollingTarget = target;

    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* view = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
        if (!lp->isNestedScrollAccepted(type)) {
            continue;
        }

        Behavior* viewBehavior = lp->getBehavior();
        if (viewBehavior != nullptr) {
            viewBehavior->onNestedScrollAccepted(*this, *view, *child, *target,nestedScrollAxes, type);
        }
    }
}

void CoordinatorLayout::onStopNestedScroll(View* target) {
    onStopNestedScroll(target, View::TYPE_TOUCH);
}

void CoordinatorLayout::onStopNestedScroll(View* target, int type) {
    mNestedScrollingParentHelper->onStopNestedScroll(target, type);

    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* view = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
        if (!lp->isNestedScrollAccepted(type)) {
            continue;
        }

        Behavior* viewBehavior = lp->getBehavior();
        if (viewBehavior != nullptr) {
            viewBehavior->onStopNestedScroll(*this, *view, *target, type);
        }
        lp->resetNestedScroll(type);
        lp->resetChangedAfterNestedScroll();
    }
    mNestedScrollingTarget = nullptr;
}

void CoordinatorLayout::onNestedScroll(View* target, int dxConsumed, int dyConsumed,
        int dxUnconsumed, int dyUnconsumed) {
    onNestedScroll(target, dxConsumed, dyConsumed, dxUnconsumed, dyUnconsumed,View::TYPE_TOUCH);
}

void CoordinatorLayout::onNestedScroll(View* target, int dxConsumed, int dyConsumed,
        int dxUnconsumed, int dyUnconsumed, int type) {
    const int childCount = getChildCount();
    bool accepted = false;

    for (int i = 0; i < childCount; i++) {
        View* view = getChildAt(i);
        if (view->getVisibility() == View::GONE) {
            // If the child is GONE, skip...
            continue;
        }

        LayoutParams*lp = (LayoutParams*) view->getLayoutParams();
        if (!lp->isNestedScrollAccepted(type)) {
            continue;
        }

        Behavior* viewBehavior = lp->getBehavior();
        if (viewBehavior != nullptr) {
            viewBehavior->onNestedScroll(*this, *view, *target, dxConsumed, dyConsumed,
                    dxUnconsumed, dyUnconsumed, type);
            accepted = true;
        }
    }

    if (accepted) {
        onChildViewsChanged(EVENT_NESTED_SCROLL);
    }
}

void CoordinatorLayout::onNestedPreScroll(View* target, int dx, int dy, int* consumed) {
    onNestedPreScroll(target, dx, dy, consumed, View::TYPE_TOUCH);
}

void CoordinatorLayout::onNestedPreScroll(View* target, int dx, int dy, int* consumed, int  type) {
    int xConsumed = 0;
    int yConsumed = 0;
    bool accepted = false;

    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* view = getChildAt(i);
        if (view->getVisibility() == View::GONE) {
            // If the child is GONE, skip...
            continue;
        }

        LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
        if (!lp->isNestedScrollAccepted(type)) {
            continue;
        }

        Behavior* viewBehavior = lp->getBehavior();
        if (viewBehavior != nullptr) {
            int mTempIntPair[2] = {0,0};
            viewBehavior->onNestedPreScroll(*this, *view, *target, dx, dy, mTempIntPair, type);

            xConsumed = dx > 0 ? std::max(xConsumed, mTempIntPair[0]) : std::min(xConsumed, mTempIntPair[0]);
            yConsumed = dy > 0 ? std::max(yConsumed, mTempIntPair[1]) : std::min(yConsumed, mTempIntPair[1]);

            accepted = true;
        }
    }

    consumed[0] = xConsumed;
    consumed[1] = yConsumed;

    if (accepted) {
        onChildViewsChanged(EVENT_NESTED_SCROLL);
    }
}

bool CoordinatorLayout::onNestedFling(View* target, float velocityX, float velocityY, bool consumed) {
    bool handled = false;

    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* view = getChildAt(i);
        if (view->getVisibility() == View::GONE) {
            // If the child is GONE, skip...
            continue;
        }

        LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
        if (!lp->isNestedScrollAccepted(View::TYPE_TOUCH)) {
            continue;
        }

        Behavior* viewBehavior = lp->getBehavior();
        if (viewBehavior != nullptr) {
            handled |= viewBehavior->onNestedFling(*this, *view, *target, velocityX, velocityY,
                    consumed);
        }
    }
    if (handled) {
        onChildViewsChanged(EVENT_NESTED_SCROLL);
    }
    return handled;
}

bool CoordinatorLayout::onNestedPreFling(View* target, float velocityX, float velocityY) {
    bool handled = false;

    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* view = getChildAt(i);
        if (view->getVisibility() == View::GONE) {
            // If the child is GONE, skip...
            continue;
        }

        LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
        if (!lp->isNestedScrollAccepted(View::TYPE_TOUCH)) {
            continue;
        }

        Behavior* viewBehavior = lp->getBehavior();
        if (viewBehavior != nullptr) {
            handled |= viewBehavior->onNestedPreFling(*this, *view, *target, velocityX, velocityY);
        }
    }
    return handled;
}

int CoordinatorLayout::getNestedScrollAxes() {
    return mNestedScrollingParentHelper->getNestedScrollAxes();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CoordinatorLayout::Behavior::setTag(View& child, void* tag) {
    LayoutParams* lp = (LayoutParams*)child.getLayoutParams();
    lp->mBehaviorTag = tag;
}

void* CoordinatorLayout::Behavior::getTag(View& child) {
    LayoutParams* lp = (LayoutParams*)child.getLayoutParams();
    return lp->mBehaviorTag;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
CoordinatorLayout::LayoutParams::LayoutParams(int width, int height)
    :MarginLayoutParams(width,height) {
    init();
}

void CoordinatorLayout::LayoutParams::init() {
    gravity = Gravity::NO_GRAVITY;
    anchorGravity = Gravity::NO_GRAVITY;
    keyline = -1;
    mAnchorId = View::NO_ID;
    insetEdge = Gravity::NO_GRAVITY;
    dodgeInsetEdges = Gravity::NO_GRAVITY;
    mBehavior = nullptr;
    mAnchorView = nullptr;
    mAnchorDirectChild = nullptr;
    mBehaviorTag = nullptr;
}

CoordinatorLayout::LayoutParams::~LayoutParams() {
    delete mBehavior;
}

CoordinatorLayout::LayoutParams::LayoutParams(Context* context, const AttributeSet& attrs)
    :MarginLayoutParams(context, attrs){
    init();
    //final TypedArray a = context.obtainStyledAttributes(attrs,
    //        R.styleable.CoordinatorLayout_Layout);

    this->gravity = attrs.getGravity("layout_gravity",Gravity::NO_GRAVITY);
    mAnchorId = attrs.getResourceId("layout_anchor",View::NO_ID);
    anchorGravity = attrs.getGravity("layout_anchorGravity",Gravity::NO_GRAVITY);

    this->keyline = attrs.getInt("layout_keyline", -1);

    insetEdge = attrs.getGravity("layout_insetEdge", Gravity::NO_GRAVITY);
    dodgeInsetEdges = attrs.getGravity("layout_dodgeInsetEdges", Gravity::NO_GRAVITY);
    mBehaviorResolved = attrs.hasAttribute("layout_behavior");
    if (mBehaviorResolved) {
        mBehavior = parseBehavior(context, attrs, attrs.getString("layout_behavior"));
    }

    if (mBehavior != nullptr) {
        // If we have a Behavior, dispatch that it has been attached
        mBehavior->onAttachedToLayoutParams(*this);
    }
}

CoordinatorLayout::LayoutParams::LayoutParams(const LayoutParams& p)
    :MarginLayoutParams(p){
    init();
}

CoordinatorLayout::LayoutParams::LayoutParams(const MarginLayoutParams& p)
    :MarginLayoutParams(p){
    init();
}

CoordinatorLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& p)
    :MarginLayoutParams(p){
    init();
}

int CoordinatorLayout::LayoutParams::getAnchorId() const{
    return mAnchorId;
}

void CoordinatorLayout::LayoutParams::setAnchorId(int id) {
    invalidateAnchor();
    mAnchorId = id;
}

CoordinatorLayout::Behavior* CoordinatorLayout::LayoutParams::getBehavior()const {
    return mBehavior;
}

void CoordinatorLayout::LayoutParams::setBehavior(Behavior* behavior) {
    if (mBehavior != behavior) {
        if (mBehavior != nullptr) {
            // First detach any old behavior
            mBehavior->onDetachedFromLayoutParams();
            delete mBehavior;
        }
        mBehavior = behavior;
        mBehaviorTag = nullptr;
        mBehaviorResolved = true;

        if (behavior != nullptr) {
            // Now dispatch that the Behavior has been attached
            behavior->onAttachedToLayoutParams(*this);
        }
    }
}

/**
 * Set the last known position rect for this child view
 * @param r the rect to set
 */
void CoordinatorLayout::LayoutParams::setLastChildRect(const Rect& r) {
    mLastChildRect = r;
}

/**
 * Get the last known position rect for this child view.
 * Note: do not mutate the result of this call.
 */
Rect CoordinatorLayout::LayoutParams::getLastChildRect() const{
    return mLastChildRect;
}

bool CoordinatorLayout::LayoutParams::checkAnchorChanged() const {
    return (mAnchorView == nullptr) && (mAnchorId != View::NO_ID);
}

bool CoordinatorLayout::LayoutParams::didBlockInteraction() {
    if (mBehavior == nullptr) {
        mDidBlockInteraction = false;
    }
    return mDidBlockInteraction;
}

bool CoordinatorLayout::LayoutParams::isBlockingInteractionBelow(CoordinatorLayout& parent, View* child) {
    if (mDidBlockInteraction) {
        return true;
    }

    mDidBlockInteraction |= mBehavior ? mBehavior->blocksInteractionBelow(parent, *child): false;
    return mDidBlockInteraction;
}

void CoordinatorLayout::LayoutParams::resetTouchBehaviorTracking() {
    mDidBlockInteraction = false;
}

void CoordinatorLayout::LayoutParams::resetNestedScroll(int type) {
    setNestedScrollAccepted(type, false);
}

void CoordinatorLayout::LayoutParams::setNestedScrollAccepted(int type, bool accept) {
    switch (type) {
    case View::TYPE_TOUCH:
        mDidAcceptNestedScrollTouch = accept;
        break;
    case View::TYPE_NON_TOUCH:
        mDidAcceptNestedScrollNonTouch = accept;
        break;
    }
}

bool CoordinatorLayout::LayoutParams::isNestedScrollAccepted(int type) {
    switch (type) {
    case View::TYPE_TOUCH:
        return mDidAcceptNestedScrollTouch;
    case View::TYPE_NON_TOUCH:
        return mDidAcceptNestedScrollNonTouch;
    }
    return false;
}

bool CoordinatorLayout::LayoutParams::getChangedAfterNestedScroll() {
    return mDidChangeAfterNestedScroll;
}

void CoordinatorLayout::LayoutParams::setChangedAfterNestedScroll(bool changed) {
    mDidChangeAfterNestedScroll = changed;
}

void CoordinatorLayout::LayoutParams::resetChangedAfterNestedScroll() {
    mDidChangeAfterNestedScroll = false;
}

bool CoordinatorLayout::LayoutParams::dependsOn(CoordinatorLayout& parent, View* child, View* dependency) {
    return (dependency == mAnchorDirectChild) || shouldDodge(dependency, parent.getLayoutDirection())
            || (mBehavior && mBehavior->layoutDependsOn(parent, *child, *dependency));
}

void CoordinatorLayout::LayoutParams::invalidateAnchor() {
    mAnchorView = mAnchorDirectChild = nullptr;
}

View* CoordinatorLayout::LayoutParams::findAnchorView(CoordinatorLayout& parent, View* forChild) {
    if (mAnchorId == View::NO_ID) {
        mAnchorView = mAnchorDirectChild = nullptr;
        return nullptr;
    }

    if ((mAnchorView == nullptr) || !verifyAnchorView(forChild, parent)) {
        resolveAnchorView(forChild, parent);
    }
    return mAnchorView;
}

void CoordinatorLayout::LayoutParams::resolveAnchorView(View* forChild,CoordinatorLayout& parent) {
    mAnchorView = parent.findViewById(mAnchorId);
    if (mAnchorView != nullptr) {
        if (mAnchorView == &parent) {
            if (parent.isInEditMode()) {
                mAnchorView = mAnchorDirectChild = nullptr;
                return;
            }
            LOGE("View can not be anchored to the the parent CoordinatorLayout");
        }

        View* directChild = mAnchorView;
        for (ViewGroup* p = mAnchorView->getParent();  p && (p != &parent); p = p->getParent()) {
            if (p == forChild) {
                if (parent.isInEditMode()) {
                    mAnchorView = mAnchorDirectChild = nullptr;
                    return;
                }
                LOGE("Anchor must not be a descendant of the anchored view");
            }
            //if (p instanceof View) {
                directChild = (View*) p;
            //}
        }
        mAnchorDirectChild = directChild;
    } else {
        if (parent.isInEditMode()) {
            mAnchorView = mAnchorDirectChild = nullptr;
            return;
        }
        LOGE("Could not find CoordinatorLayout descendant view with id %d o anchor view %p",mAnchorId,forChild);
    }
}

bool CoordinatorLayout::LayoutParams::verifyAnchorView(View* forChild, CoordinatorLayout& parent) {
    if (mAnchorView->getId() != mAnchorId) {
        return false;
    }

    View* directChild = mAnchorView;
    for (ViewGroup* p = mAnchorView->getParent();p != &parent; p = p->getParent()) {
        if (p == nullptr || p == forChild) {
            mAnchorView = mAnchorDirectChild = nullptr;
            return false;
        }
        //if (p instanceof View) {
            directChild = (View*) p;
        //}
    }
    mAnchorDirectChild = directChild;
    return true;
}

bool CoordinatorLayout::LayoutParams::shouldDodge(View* other, int layoutDirection) {
    LayoutParams* lp = (LayoutParams*) other->getLayoutParams();
    const int absInset = Gravity::getAbsoluteGravity(lp->insetEdge, layoutDirection);
    return absInset != Gravity::NO_GRAVITY && (absInset &
            Gravity::getAbsoluteGravity(dodgeInsetEdges, layoutDirection)) == absInset;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void CoordinatorLayout::onRestoreInstanceState(Parcelable& state) {
#if 0
    if (!(state instanceof SavedState)) {
        ViewGroup::onRestoreInstanceState(state);
        return;
    }

    SavedState ss = (SavedState) state;
    ViewGroup::onRestoreInstanceState(ss.getSuperState());

    SparseArray<Parcelable> behaviorStates = ss.behaviorStates;

    for (int i = 0, count = getChildCount(); i < count; i++) {
        View* child = getChildAt(i);
        int childId = child.getId();
        LayoutParams* lp = getResolvedLayoutParams(child);
        Behavior* b = lp->getBehavior();

        if (childId != NO_ID && b != null) {
            Parcelable savedState = behaviorStates.get(childId);
            if (savedState != null) {
                b.onRestoreInstanceState(this, child, savedState);
            }
        }
    }
#endif
}

Parcelable* CoordinatorLayout::onSaveInstanceState() {
#if 0
    SavedState ss = new SavedState(super.onSaveInstanceState());
    SparseArray<Parcelable> behaviorStates ;
    for (int i = 0, count = getChildCount(); i < count; i++) {
        final View child = getChildAt(i);
        final int childId = child.getId();
        final LayoutParams lp = (LayoutParams) child.getLayoutParams();
        final Behavior b = lp.getBehavior();

        if (childId != NO_ID && b != null) {
            // If the child has an ID and a Behavior, let it save some state...
            Parcelable state = b.onSaveInstanceState(this, child);
            if (state != null) {
                behaviorStates.append(childId, state);
            }
        }
    }
    ss.behaviorStates = behaviorStates;
    return ss;
#else
    return nullptr;
#endif
}

 bool  CoordinatorLayout::requestChildRectangleOnScreen(View* child, Rect& rectangle, bool immediate) {
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    Behavior* behavior = lp->getBehavior();

    if (behavior && behavior->onRequestChildRectangleOnScreen(*this, *child, rectangle, immediate)) {
        return true;
    }

    return ViewGroup::requestChildRectangleOnScreen(child, rectangle, immediate);
}

void CoordinatorLayout::setupForInsets() {

    if (this->getFitsSystemWindows()) {
        if (mApplyWindowInsetsListener == nullptr) {
            mApplyWindowInsetsListener = [this](View& v, WindowInsets& insets) {
                    return setWindowInsets(insets);
                };
        }
        // First apply the insets listener
        this->setOnApplyWindowInsetsListener(mApplyWindowInsetsListener);

        // Now set the sys ui flags to enable us to lay out in the window insets
        this->setSystemUiVisibility(View::SYSTEM_UI_FLAG_LAYOUT_STABLE | View::SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
    } else {
        this->setOnApplyWindowInsetsListener(nullptr);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
#if 0
protected static class SavedState extends AbsSavedState {
    SparseArray<Parcelable> behaviorStates;

    public SavedState(Parcel source, ClassLoader loader) {
        super(source, loader);

        final int size = source.readInt();

        final int[] ids = new int[size];
        source.readIntArray(ids);

        final Parcelable[] states = source.readParcelableArray(loader);

        behaviorStates = new SparseArray<>(size);
        for (int i = 0; i < size; i++) {
            behaviorStates.append(ids[i], states[i]);
        }
    }

    public SavedState(Parcelable superState) {
        super(superState);
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        super.writeToParcel(dest, flags);

        final int size = behaviorStates != null ? behaviorStates.size() : 0;
        dest.writeInt(size);

        final int[] ids = new int[size];
        final Parcelable[] states = new Parcelable[size];

        for (int i = 0; i < size; i++) {
            ids[i] = behaviorStates.keyAt(i);
            states[i] = behaviorStates.valueAt(i);
        }
        dest.writeIntArray(ids);
        dest.writeParcelableArray(states, flags);

    }

public static final Creator<SavedState> CREATOR =
        new ClassLoaderCreator<SavedState>() {
            @Override
            public SavedState createFromParcel(Parcel in, ClassLoader loader) {
                return new SavedState(in, loader);
            }

            @Override
            public SavedState createFromParcel(Parcel in) {
                return new SavedState(in, null);
            }

            @Override
            public SavedState[] newArray(int size) {
                return new SavedState[size];
            }
        };
};/*endof class SavedState*/
#endif
}
/*endof namespace*/
