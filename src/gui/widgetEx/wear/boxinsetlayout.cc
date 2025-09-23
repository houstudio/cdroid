#include <widgetEx/wear/boxinsetlayout.h>
namespace cdroid{

BoxInsetLayout::BoxInsetLayout(Context* context,const AttributeSet& attrs)
    :ViewGroup(context, attrs){
    // make sure we have a foreground padding object
    /*if (mForegroundPadding == nullptr) {
        mForegroundPadding = new Rect();
    }
    if (mInsets == null) {
        mInsets = new Rect();
    }*/
    mScreenHeight= context->getDisplayMetrics().heightPixels;
    mScreenWidth = context->getDisplayMetrics().widthPixels;
}

void BoxInsetLayout::setForeground(Drawable* drawable) {
    ViewGroup::setForeground(drawable);
    mForegroundDrawable = drawable;
    /*if (mForegroundPadding == nullptr) {
        mForegroundPadding = new Rect();
    }*/
    if (mForegroundDrawable != nullptr) {
        drawable->getPadding(mForegroundPadding);
    }
}

BoxInsetLayout::LayoutParams* BoxInsetLayout::generateLayoutParams(const AttributeSet& attrs) const{
    return new BoxInsetLayout::LayoutParams(getContext(), attrs);
}

void BoxInsetLayout::onAttachedToWindow() {
    ViewGroup::onAttachedToWindow();
    mIsRound = true;//getResources().getConfiguration().isScreenRound();
    WindowInsets insets = getRootWindowInsets();
    mInsets.set(insets.getSystemWindowInsetLeft(), insets.getSystemWindowInsetTop(),
            insets.getSystemWindowInsetRight(), insets.getSystemWindowInsetBottom());
}

void BoxInsetLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    const int count = getChildCount();
    // find max size
    int maxWidth = 0;
    int maxHeight = 0;
    int childState = 0;
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() != GONE) {
            const LayoutParams* lp = (BoxInsetLayout::LayoutParams*) child->getLayoutParams();
            int marginLeft = 0;
            int marginRight = 0;
            int marginTop = 0;
            int marginBottom = 0;
            if (mIsRound) {
                // round screen, check boxed, don't use margins on boxed
                if ((lp->boxedEdges & LayoutParams::BOX_LEFT) == 0) {
                    marginLeft = lp->leftMargin;
                }
                if ((lp->boxedEdges & LayoutParams::BOX_RIGHT) == 0) {
                    marginRight = lp->rightMargin;
                }
                if ((lp->boxedEdges & LayoutParams::BOX_TOP) == 0) {
                    marginTop = lp->topMargin;
                }
                if ((lp->boxedEdges & LayoutParams::BOX_BOTTOM) == 0) {
                    marginBottom = lp->bottomMargin;
                }
            } else {
                // rectangular, ignore boxed, use margins
                marginLeft = lp->leftMargin;
                marginTop = lp->topMargin;
                marginRight = lp->rightMargin;
                marginBottom = lp->bottomMargin;
            }
            measureChildWithMargins(child, widthMeasureSpec, 0, heightMeasureSpec, 0);
            maxWidth = std::max(maxWidth, child->getMeasuredWidth() + marginLeft + marginRight);
            maxHeight = std::max(maxHeight, child->getMeasuredHeight() + marginTop + marginBottom);
            childState = combineMeasuredStates(childState, child->getMeasuredState());
        }
    }
    // Account for padding too
    maxWidth += getPaddingLeft() + mForegroundPadding.left + getPaddingRight()
            + mForegroundPadding.width;
    maxHeight += getPaddingTop() + mForegroundPadding.top + getPaddingBottom()
            + mForegroundPadding.height;

    // Check against our minimum height and width
    maxHeight = std::max(maxHeight, getSuggestedMinimumHeight());
    maxWidth = std::max(maxWidth, getSuggestedMinimumWidth());

    // Check against our foreground's minimum height and width
    if (mForegroundDrawable != nullptr) {
        maxHeight = std::max(maxHeight, mForegroundDrawable->getMinimumHeight());
        maxWidth = std::max(maxWidth, mForegroundDrawable->getMinimumWidth());
    }

    const int measuredWidth = resolveSizeAndState(maxWidth, widthMeasureSpec, childState);
    const int measuredHeight = resolveSizeAndState(maxHeight, heightMeasureSpec,
            childState << MEASURED_HEIGHT_STATE_SHIFT);
    setMeasuredDimension(measuredWidth, measuredHeight);

    // determine boxed inset
    const int boxInset = calculateInset(measuredWidth, measuredHeight);
    // adjust the the children measures, if necessary
    for (int i = 0; i < count; i++) {
        measureChild(widthMeasureSpec, heightMeasureSpec, boxInset, i);
    }
}

void BoxInsetLayout::onLayout(bool changed, int left, int top, int layoutWidth, int layoutHeight) {
    const int count = getChildCount();

    const int parentLeft = getPaddingLeft() + mForegroundPadding.left;
    const int parentRight = layoutWidth - getPaddingRight() - mForegroundPadding.width;

    const int parentTop = getPaddingTop() + mForegroundPadding.top;
    const int parentBottom = layoutHeight - getPaddingBottom() - mForegroundPadding.height;

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() != GONE) {
            const LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

            const int width = child->getMeasuredWidth();
            const int height = child->getMeasuredHeight();

            int childLeft;
            int childTop;

            int gravity = lp->gravity;
            if (gravity == -1) {
                gravity = DEFAULT_CHILD_GRAVITY;
            }

            const int layoutDirection = getLayoutDirection();
            const int absoluteGravity = Gravity::getAbsoluteGravity(gravity, layoutDirection);
            const int verticalGravity = gravity & Gravity::VERTICAL_GRAVITY_MASK;
            const int horizontalGravity = gravity & Gravity::HORIZONTAL_GRAVITY_MASK;
            int desiredInset = calculateInset(getMeasuredWidth(), getMeasuredHeight());

            // If the child's width is match_parent then we can ignore gravity.
            const int leftChildMargin = calculateChildLeftMargin(lp, horizontalGravity, desiredInset);
            const int rightChildMargin = calculateChildRightMargin(lp, horizontalGravity, desiredInset);
            if (lp->width == LayoutParams::MATCH_PARENT) {
                childLeft = parentLeft + leftChildMargin;
            } else {
                switch (absoluteGravity & Gravity::HORIZONTAL_GRAVITY_MASK) {
                case Gravity::CENTER_HORIZONTAL:
                    childLeft = parentLeft + (parentRight - parentLeft - width) / 2
                            + leftChildMargin - rightChildMargin;
                    break;
                case Gravity::RIGHT:
                    childLeft = parentRight - width - rightChildMargin;
                    break;
                case Gravity::LEFT:
                default:
                    childLeft = parentLeft + leftChildMargin;
                }
            }

            // If the child's height is match_parent then we can ignore gravity.
            const int topChildMargin = calculateChildTopMargin(lp, verticalGravity, desiredInset);
            const int bottomChildMargin = calculateChildBottomMargin(lp, verticalGravity, desiredInset);
            if (lp->height == LayoutParams::MATCH_PARENT) {
                childTop = parentTop + topChildMargin;
            } else {
                switch (verticalGravity) {
                case Gravity::CENTER_VERTICAL:
                    childTop = parentTop + (parentBottom - parentTop - height) / 2
                            + topChildMargin - bottomChildMargin;
                    break;
                case Gravity::BOTTOM:
                    childTop = parentBottom - height - bottomChildMargin;
                    break;
                case Gravity::TOP:
                default:
                    childTop = parentTop + topChildMargin;
                }
            }
            child->layout(childLeft, childTop, width, height);
        }
    }
}

bool BoxInsetLayout::checkLayoutParams(const ViewGroup::LayoutParams* p) const{
    return dynamic_cast<const LayoutParams*>(p);
}

BoxInsetLayout::LayoutParams* BoxInsetLayout::generateLayoutParams(const ViewGroup::LayoutParams* p) const{
    return new LayoutParams(*p);
}

void BoxInsetLayout::measureChild(int widthMeasureSpec, int heightMeasureSpec, int desiredMinInset, int i) {
    View* child = getChildAt(i);
    const LayoutParams* childLayoutParams = (LayoutParams*) child->getLayoutParams();

    int gravity = childLayoutParams->gravity;
    if (gravity == -1) {
        gravity = DEFAULT_CHILD_GRAVITY;
    }
    const int verticalGravity = gravity & Gravity::VERTICAL_GRAVITY_MASK;
    const int horizontalGravity = gravity & Gravity::HORIZONTAL_GRAVITY_MASK;

    int childWidthMeasureSpec;
    int childHeightMeasureSpec;

    int leftParentPadding = getPaddingLeft() + mForegroundPadding.left;
    int rightParentPadding = getPaddingRight() + mForegroundPadding.width;
    int topParentPadding = getPaddingTop() + mForegroundPadding.top;
    int bottomParentPadding = getPaddingBottom() + mForegroundPadding.height;

    // adjust width
    const int totalWidthMargin = leftParentPadding + rightParentPadding + calculateChildLeftMargin(
            childLayoutParams, horizontalGravity, desiredMinInset) + calculateChildRightMargin(
            childLayoutParams, horizontalGravity, desiredMinInset);

    // adjust height
    const int totalHeightMargin = topParentPadding + bottomParentPadding + calculateChildTopMargin(
            childLayoutParams, verticalGravity, desiredMinInset) + calculateChildBottomMargin(
            childLayoutParams, verticalGravity, desiredMinInset);

    childWidthMeasureSpec = getChildMeasureSpec(widthMeasureSpec, totalWidthMargin,
            childLayoutParams->width);
    childHeightMeasureSpec = getChildMeasureSpec(heightMeasureSpec, totalHeightMargin,
            childLayoutParams->height);

    const int maxAllowedWidth = getMeasuredWidth() - totalWidthMargin;
    const int maxAllowedHeight = getMeasuredHeight() - totalHeightMargin;
    if (child->getMeasuredWidth() > maxAllowedWidth
            || child->getMeasuredHeight() > maxAllowedHeight) {
        child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
    }
}

int BoxInsetLayout::calculateChildLeftMargin(const LayoutParams* lp, int horizontalGravity, int desiredMinInset) const{
    if (mIsRound && ((lp->boxedEdges & LayoutParams::BOX_LEFT) != 0)) {
        if (lp->width == LayoutParams::MATCH_PARENT || horizontalGravity == Gravity::LEFT) {
            return lp->leftMargin + desiredMinInset;
        }
    }
    return lp->leftMargin;
}

int BoxInsetLayout::calculateChildRightMargin(const LayoutParams* lp, int horizontalGravity, int desiredMinInset) const{
    if (mIsRound && ((lp->boxedEdges & LayoutParams::BOX_RIGHT) != 0)) {
        if ((lp->width == LayoutParams::MATCH_PARENT) || (horizontalGravity == Gravity::RIGHT)) {
            return lp->rightMargin + desiredMinInset;
        }
    }
    return lp->rightMargin;
}

int BoxInsetLayout::calculateChildTopMargin(const LayoutParams* lp, int verticalGravity, int desiredMinInset) const{
    if (mIsRound && ((lp->boxedEdges & LayoutParams::BOX_TOP) != 0)) {
        if ((lp->height == LayoutParams::MATCH_PARENT) || (verticalGravity == Gravity::TOP)) {
            return lp->topMargin + desiredMinInset;
        }
    }
    return lp->topMargin;
}

int BoxInsetLayout::calculateChildBottomMargin(const LayoutParams* lp, int verticalGravity, int desiredMinInset) const{
    if (mIsRound && ((lp->boxedEdges & LayoutParams::BOX_BOTTOM) != 0)) {
        if ((lp->height == LayoutParams::MATCH_PARENT) || (verticalGravity == Gravity::BOTTOM)) {
            return lp->bottomMargin + desiredMinInset;
        }
    }
    return lp->bottomMargin;
}

int BoxInsetLayout::calculateInset(int measuredWidth, int measuredHeight) {
    const int rightEdge = std::min(measuredWidth, mScreenWidth);
    const int bottomEdge = std::min(measuredHeight, mScreenHeight);
    return (int) (FACTOR * std::max(rightEdge, bottomEdge));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BoxInsetLayout::LayoutParams::LayoutParams(Context* context, const AttributeSet& attrs)
    :FrameLayout::LayoutParams(context, attrs){
    /*TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.BoxInsetLayout_Layout,0, 0);
    int boxedEdgesResourceKey = R.styleable.BoxInsetLayout_Layout_layout_boxedEdges;
    if (!a.hasValueOrEmpty(R.styleable.BoxInsetLayout_Layout_layout_boxedEdges)){
        boxedEdgesResourceKey = R.styleable.BoxInsetLayout_Layout_boxedEdges;
    }*/
    boxedEdges = attrs.getInt("boxedEdges",std::unordered_map<std::string,int>{
            {"none",BOX_NONE},
            {"left",BOX_LEFT},
            {"top",BOX_TOP},
            {"right",BOX_RIGHT},
            {"bottom",BOX_BOTTOM}
            }, BOX_NONE);
}

BoxInsetLayout::LayoutParams::LayoutParams(int width, int height)
    :FrameLayout::LayoutParams(width, height){
}

BoxInsetLayout::LayoutParams::LayoutParams(int width, int height, int gravity)
    :FrameLayout::LayoutParams(width, height, gravity){
}


BoxInsetLayout::LayoutParams::LayoutParams(int width, int height, int gravity, int boxed)
    :FrameLayout::LayoutParams(width, height, gravity){
    boxedEdges = boxed;
}

BoxInsetLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
    :FrameLayout::LayoutParams(source){
}

BoxInsetLayout::LayoutParams::LayoutParams(const ViewGroup::MarginLayoutParams& source)
    :FrameLayout::LayoutParams(source){
}

BoxInsetLayout::LayoutParams::LayoutParams(const FrameLayout::LayoutParams& source)
    :FrameLayout::LayoutParams(source){
}

BoxInsetLayout::LayoutParams::LayoutParams(const LayoutParams& source)
    :FrameLayout::LayoutParams(source){
    this->boxedEdges = source.boxedEdges;
    this->gravity = source.gravity;
}
}/*endofnamespace*/
