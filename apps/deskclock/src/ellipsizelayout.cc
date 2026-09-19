#include <ellipsizelayout.h>

#include <widget/textview.h>

namespace cdroid {
namespace deskclock {

EllipsizeLayout::EllipsizeLayout(Context* context, const AttributeSet* attrs)
    : LinearLayout(context, attrs) {
}

void EllipsizeLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    if (getOrientation() == HORIZONTAL
            && View::MeasureSpec::getMode(widthMeasureSpec) == View::MeasureSpec::EXACTLY) {
        int totalLength = 0;
        bool outOfSpec = false;
        TextView* ellipsizeView = nullptr;
        const int count = getChildCount();
        const int parentWidth = View::MeasureSpec::getSize(widthMeasureSpec);
        const int queryWidthMeasureSpec = View::MeasureSpec::makeMeasureSpec(
                View::MeasureSpec::getSize(widthMeasureSpec), View::MeasureSpec::UNSPECIFIED);

        int ii = 0;
        while (ii < count && !outOfSpec) {
            View* child = getChildAt(ii);
            if (child != nullptr && child->getVisibility() != View::GONE) {
                if (TextView* tv = dynamic_cast<TextView*>(child)) {
                    if (tv->getEllipsize() != TextUtils::TruncateAt::NONE) {
                        if (ellipsizeView == nullptr) {
                            ellipsizeView = tv;
                            ellipsizeView->setMaxWidth(INT_MAX);
                        } else {
                            outOfSpec = true;
                        }
                    }
                }
                measureChildWithMargins(child, queryWidthMeasureSpec, 0, heightMeasureSpec, 0);

                LayoutParams* layoutParams = (LayoutParams*) child->getLayoutParams();
                if (layoutParams != nullptr) {
                    outOfSpec = outOfSpec || layoutParams->weight > 0.0f;
                    totalLength += child->getMeasuredWidth()
                            + layoutParams->leftMargin + layoutParams->rightMargin;
                } else {
                    outOfSpec = true;
                }
            }
            ++ii;
        }
        outOfSpec = outOfSpec || ellipsizeView == nullptr || totalLength == 0;

        if (!outOfSpec && totalLength > parentWidth) {
            int maxWidth = ellipsizeView->getMeasuredWidth() - (totalLength - parentWidth);
            const int minWidth = 0;
            if (maxWidth < minWidth) {
                maxWidth = minWidth;
            }
            ellipsizeView->setMaxWidth(maxWidth);
        }
    }

    LinearLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
}

} // namespace deskclock

typedef cdroid::deskclock::EllipsizeLayout EllipsizeLayout;
DECLARE_WIDGET2(EllipsizeLayout, "EllipsizeLayout");

} // namespace cdroid
