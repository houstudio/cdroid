#include <widget/tablerow.h>
#include <cdlog.h>
namespace cdroid{

DECLARE_WIDGET(TableRow)

TableRow::LayoutParams::LayoutParams()
    :LinearLayout::LayoutParams(MATCH_PARENT, WRAP_CONTENT){
    column= -1;
    span  = 1;
}

TableRow::LayoutParams::LayoutParams(int column):LayoutParams(){
    this->column=column;
}

TableRow::LayoutParams::LayoutParams(Context* c,const AttributeSet&attrs)
    :LinearLayout::LayoutParams(c,attrs){
    column= attrs.getInt("layout_column",-1);
    span  = attrs.getInt("layout_span",1);
    if(span<1)span=1;
}

TableRow::LayoutParams::LayoutParams(int w, int h)
    :LinearLayout::LayoutParams(w,h){
    column= -1;
    span  = 1;
}

TableRow::LayoutParams::LayoutParams(int w, int h, float initWeight)
    :LinearLayout::LayoutParams(w,h,initWeight){
    column= -1;
    span  = 1;
}

TableRow::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& p)
    :LinearLayout::LayoutParams(p){
    column= -1;
    span  = 1;
}

TableRow::LayoutParams::LayoutParams(const MarginLayoutParams& source)
    :LinearLayout::LayoutParams(source){
    column= -1;
    span  = 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

TableRow::TableRow(int w,int h):LinearLayout(w,h){
    initTableRow();
}

TableRow::TableRow(Context* context,const AttributeSet& attrs)
  :LinearLayout(context, attrs){
    initTableRow();
}

void TableRow::initTableRow() {
    OnHierarchyChangeListener oldListener = mOnHierarchyChangeListener;
    mChildrenTracker.onChildViewAdded  = std::bind(&TableRow::onChildViewAdded,this,std::placeholders::_1,std::placeholders::_2);
    mChildrenTracker.onChildViewRemoved= std::bind(&TableRow::onChildViewRemoved,this,std::placeholders::_1,std::placeholders::_2);
    if (oldListener.onChildViewAdded||oldListener.onChildViewRemoved) {
        mExtHCL = oldListener;
    }
    ViewGroup::setOnHierarchyChangeListener(mChildrenTracker);
}

void TableRow::setOnHierarchyChangeListener(const OnHierarchyChangeListener& listener) {
    mExtHCL = listener;
}

void TableRow::setColumnCollapsed(int columnIndex, bool collapsed) {
    View* child = getVirtualChildAt(columnIndex);
    if (child != nullptr) {
        child->setVisibility(collapsed ? GONE : VISIBLE);
    }
}

void TableRow::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // enforce horizontal layout
    measureHorizontal(widthMeasureSpec, heightMeasureSpec);
}

void TableRow::onLayout(bool changed, int l, int t, int w, int h) {
    // enforce horizontal layout
    layoutHorizontal(l, t, w, h);
}

View* TableRow::getVirtualChildAt(int i){
    if (mColumnToChildIndex.size()==0) {
        mapIndexAndColumns();
    }
    const int deflectedIndex = mColumnToChildIndex.get(i,-1);
    if (deflectedIndex != -1) {
        return getChildAt(deflectedIndex);
    }
    return nullptr;
}

int TableRow::getVirtualChildCount() {
    if (mColumnToChildIndex.size()==0) {
        mapIndexAndColumns();
    }
    return mNumColumns;
}

std::string TableRow::getAccessibilityClassName()const{
    return "TableRow";
}

void TableRow::mapIndexAndColumns() {
    if (mColumnToChildIndex.size() == 0) {
        int virtualCount = 0;
        int count = getChildCount();

        for (int i = 0; i < count; i++) {
            View* child = getChildAt(i);
            LayoutParams* layoutParams = (LayoutParams*) child->getLayoutParams();
            if (layoutParams->column >= virtualCount) {
                virtualCount = layoutParams->column;
            }

            for (int j = 0; j < layoutParams->span; j++) {
                mColumnToChildIndex.put(virtualCount++,i);
            }
        }

        mNumColumns = virtualCount;
    }
}

void TableRow::onChildViewAdded(View& parent, View* child) {
    // dirties the index to column map
    mColumnToChildIndex.clear();// = null;

    if (mExtHCL.onChildViewAdded != nullptr) {
        mExtHCL.onChildViewAdded(parent, child);
    }
}

void TableRow::onChildViewRemoved(View& parent, View* child) {
    // dirties the index to column map
    mColumnToChildIndex.clear();// = null;

    if (mExtHCL.onChildViewRemoved != nullptr) {
        mExtHCL.onChildViewRemoved(parent, child);
    }
}

int TableRow::measureNullChild(int childIndex) {
    return mConstrainedColumnWidths[childIndex];
}

void TableRow::measureChildBeforeLayout(View* child, int childIndex,int widthMeasureSpec,
        int totalWidth,int heightMeasureSpec, int totalHeight) {
    if (mConstrainedColumnWidths.size()) {
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

        int measureMode = MeasureSpec::EXACTLY;
        int columnWidth = 0;

        const int span = lp->span;
        for (int i = 0; i < span; i++) {
            columnWidth += mConstrainedColumnWidths[childIndex + i];
        }

        const int gravity = lp->gravity;
        const bool isHorizontalGravity = Gravity::isHorizontal(gravity);

        if (isHorizontalGravity) {
            measureMode = MeasureSpec::AT_MOST;
        }

        // no need to care about padding here,
        // ViewGroup.getChildMeasureSpec() would get rid of it anyway
        // because of the EXACTLY measure spec we use
        const int childWidthMeasureSpec = MeasureSpec::makeMeasureSpec(
                    std::max(0, columnWidth - lp->leftMargin - lp->rightMargin), measureMode);
        const int childHeightMeasureSpec = getChildMeasureSpec(heightMeasureSpec,
                    mPaddingTop + mPaddingBottom + lp->topMargin +
                    lp->bottomMargin + totalHeight, lp->height);

        child->measure(childWidthMeasureSpec, childHeightMeasureSpec);

        if (isHorizontalGravity) {
            const int childWidth = child->getMeasuredWidth();
            lp->mOffset[LayoutParams::LOCATION_NEXT] = columnWidth - childWidth;

            const int layoutDirection = getLayoutDirection();
            const int absoluteGravity = Gravity::getAbsoluteGravity(gravity, layoutDirection);
            switch (absoluteGravity & Gravity::HORIZONTAL_GRAVITY_MASK) {
            case Gravity::LEFT:
                // don't offset on X axis
                break;
            case Gravity::RIGHT:
                lp->mOffset[LayoutParams::LOCATION] = lp->mOffset[LayoutParams::LOCATION_NEXT];
                break;
            case Gravity::CENTER_HORIZONTAL:
                lp->mOffset[LayoutParams::LOCATION] = lp->mOffset[LayoutParams::LOCATION_NEXT] / 2;
                break;
            }
        } else {
            lp->mOffset[LayoutParams::LOCATION] = lp->mOffset[LayoutParams::LOCATION_NEXT] = 0;
        }
    } else {
        // fail silently when column widths are not available
        LinearLayout::measureChildBeforeLayout(child, childIndex, widthMeasureSpec,
                    totalWidth, heightMeasureSpec, totalHeight);
    }
}

int TableRow::getChildrenSkipCount(View* child, int index) {
    LayoutParams* layoutParams = (LayoutParams*) child->getLayoutParams();
    // when the span is 1 (default), we need to skip 0 child
    return layoutParams->span - 1;
}

int TableRow::getLocationOffset(View* child) {
    return ((LayoutParams*) child->getLayoutParams())->mOffset[LayoutParams::LOCATION];
}

int TableRow::getNextLocationOffset(View* child) {
    return ((LayoutParams*) child->getLayoutParams())->mOffset[LayoutParams::LOCATION_NEXT];
}

std::vector<int> TableRow::getColumnsWidths(int widthMeasureSpec, int heightMeasureSpec) {
    int numColumns = getVirtualChildCount();
    if (numColumns != mColumnWidths.size()) {
        mColumnWidths.resize(numColumns);
    }

    for (int i = 0; i < numColumns; i++) {
        View* child = getVirtualChildAt(i);
        if (child != nullptr && child->getVisibility() != GONE) {
            LayoutParams* layoutParams = (LayoutParams*) child->getLayoutParams();
            if (layoutParams->span == 1) {
                int spec;
                switch (layoutParams->width) {
                case LayoutParams::WRAP_CONTENT:
                    spec = getChildMeasureSpec(widthMeasureSpec, 0, LayoutParams::WRAP_CONTENT);
                    break;
                case LayoutParams::MATCH_PARENT:
                    spec = MeasureSpec::makeSafeMeasureSpec(MeasureSpec::getSize(heightMeasureSpec),
                            MeasureSpec::UNSPECIFIED);
                    break;
                default:
                    spec = MeasureSpec::makeMeasureSpec(layoutParams->width, MeasureSpec::EXACTLY);
                }
                child->measure(spec, spec);

                int width = child->getMeasuredWidth() + layoutParams->leftMargin +layoutParams->rightMargin;
                mColumnWidths[i] = width;
            } else {
                mColumnWidths[i] = 0;
            }
        } else {
            mColumnWidths[i] = 0;
        }
    }

    return mColumnWidths;
}

void TableRow::setColumnsWidthConstraints(const std::vector<int>& columnWidths) {
    if (columnWidths.size() < getVirtualChildCount()) {
        LOGE("columnWidths should be >= getVirtualChildCount()");
    }
    mConstrainedColumnWidths = columnWidths;
}

TableRow::LayoutParams* TableRow::generateLayoutParams(const AttributeSet& attrs)const {
    return new LayoutParams(getContext(), attrs);
}

TableRow::LayoutParams* TableRow::generateDefaultLayoutParams()const {
    return new LayoutParams();
}

bool TableRow::checkLayoutParams(const ViewGroup::LayoutParams* p)const {
    return dynamic_cast<const LayoutParams*>(p);
}

TableRow::LayoutParams*TableRow::generateLayoutParams(const ViewGroup::LayoutParams* p)const {
    return new LayoutParams(*p);
}

}
