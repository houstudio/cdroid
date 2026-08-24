#include <widget/internal_R.h>
#include <view/layoutparams.h>
#include <widget/framework_styleable.h>
#include <core/assets.h>
#include <view/viewgroup.h>
#include <cdlog.h>
namespace cdroid{
using namespace cdroid::internal;

LayoutParams::LayoutParams(){
    // AOSP LayoutParams() defaults - width/height left uninitialized here were
    // read downstream (e.g. PopupWindow::createDecorView's WRAP_CONTENT check)
    // with garbage (valgrind: conditional jump on uninitialised value).
    width = WRAP_CONTENT;
    height= WRAP_CONTENT;
    layoutAnimationParameters =nullptr;
}

LayoutParams::LayoutParams(Context* c,const AttributeSet& attrs):LayoutParams(){
    auto ta = c->obtainStyledAttributes(attrs, R::styleable::Layout);
    setBaseAttributes(*ta, R::styleable::Layout_layout_width, R::styleable::Layout_layout_height);
}

LayoutParams::LayoutParams(int w, int h):LayoutParams(){
    width = w;
    height= h;
}

LayoutParams::LayoutParams(const LayoutParams& source):LayoutParams(){
    width = source.width;
    height= source.height;
}

LayoutParams::~LayoutParams(){
    delete layoutAnimationParameters;
}

void LayoutParams::setBaseAttributes(const TypedArray& a, int widthAttr, int heightAttr){
    width = a.getLayoutDimension(widthAttr, WRAP_CONTENT);
    height= a.getLayoutDimension(heightAttr,WRAP_CONTENT);
}

void LayoutParams::resolveLayoutDirection(int layoutDirection){
}

void LayoutParams::onDebugDraw(View&view, Canvas&canvas){
}

const std::string LayoutParams::sizeToString(int size) {
    switch(size){
    case WRAP_CONTENT: return "wrap-content";
    case MATCH_PARENT: return "match-parent";
    default:return std::to_string(size);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
MarginLayoutParams::MarginLayoutParams(Context*c,const AttributeSet& attrs)
   :LayoutParams(c,attrs){
    auto a = c->obtainStyledAttributes(attrs, R::styleable::MarginLayout);

    const int margin = a->getDimensionPixelSize(R::styleable::MarginLayout_layout_margin, -1);
    mMarginFlags = 0;
    if(margin>=0){
        leftMargin = topMargin = rightMargin = bottomMargin = margin;
        startMargin= endMargin = DEFAULT_MARGIN_RELATIVE;
    }else{
        int horizontalMargin = a->getDimensionPixelSize(R::styleable::MarginLayout_layout_marginHorizontal, -1);
        int verticalMargin   = a->getDimensionPixelSize(R::styleable::MarginLayout_layout_marginVertical, -1);
        if(horizontalMargin>=0){
            leftMargin= rightMargin = horizontalMargin;
        }else{
            leftMargin = a->getDimensionPixelSize(R::styleable::MarginLayout_layout_marginLeft, UNDEFINED_MARGIN);
            rightMargin= a->getDimensionPixelSize(R::styleable::MarginLayout_layout_marginRight,UNDEFINED_MARGIN);
            if(leftMargin==UNDEFINED_MARGIN){
                mMarginFlags |= LEFT_MARGIN_UNDEFINED_MASK;
                leftMargin = DEFAULT_MARGIN_RESOLVED;
            }
            if (rightMargin == UNDEFINED_MARGIN) {
                mMarginFlags |= RIGHT_MARGIN_UNDEFINED_MASK;
                rightMargin = DEFAULT_MARGIN_RESOLVED;
            }
        }
        startMargin = a->getDimensionPixelSize(R::styleable::MarginLayout_layout_marginStart,DEFAULT_MARGIN_RELATIVE);
        endMargin   = a->getDimensionPixelSize(R::styleable::MarginLayout_layout_marginEnd,  DEFAULT_MARGIN_RELATIVE);
        if(verticalMargin >=0 ){
            topMargin = bottomMargin = verticalMargin;
        }else{
            topMargin   = a->getDimensionPixelSize(R::styleable::MarginLayout_layout_marginTop,   DEFAULT_MARGIN_RESOLVED);
            bottomMargin= a->getDimensionPixelSize(R::styleable::MarginLayout_layout_marginBottom,DEFAULT_MARGIN_RESOLVED);
        }
        if (isMarginRelative()) {
            mMarginFlags |= NEED_RESOLUTION_MASK;
        }
    }
    // CDROID: no ApplicationInfo RTL support flag; always resolved (no RTL_COMPATIBILITY_MODE_MASK).
    mMarginFlags |= View::LAYOUT_DIRECTION_LTR;
}

MarginLayoutParams::MarginLayoutParams(int width, int height)
	:LayoutParams(width, height){
    mMarginFlags=0;
    leftMargin = topMargin   = 0;
    rightMargin= bottomMargin= 0;
    startMargin= endMargin = DEFAULT_MARGIN_RELATIVE;
    mMarginFlags |= LEFT_MARGIN_UNDEFINED_MASK;
    mMarginFlags |= RIGHT_MARGIN_UNDEFINED_MASK;
    mMarginFlags &= ~NEED_RESOLUTION_MASK;
    mMarginFlags &= ~RTL_COMPATIBILITY_MODE_MASK;
}

MarginLayoutParams::MarginLayoutParams(const LayoutParams& source)
	:LayoutParams(source){
    mMarginFlags=0;
    leftMargin = topMargin =0;
    rightMargin= bottomMargin=0;
    startMargin= endMargin = DEFAULT_MARGIN_RELATIVE;

    mMarginFlags |= LEFT_MARGIN_UNDEFINED_MASK;
    mMarginFlags |= RIGHT_MARGIN_UNDEFINED_MASK;

    mMarginFlags &= ~NEED_RESOLUTION_MASK;
    mMarginFlags &= ~RTL_COMPATIBILITY_MODE_MASK;
}

MarginLayoutParams::MarginLayoutParams(const MarginLayoutParams& source){
    width = source.width;
    height = source.height;

    leftMargin = source.leftMargin;
    topMargin = source.topMargin;
    rightMargin = source.rightMargin;
    bottomMargin = source.bottomMargin;
    startMargin = source.startMargin;
    endMargin = source.endMargin;

    mMarginFlags = source.mMarginFlags;
}

void MarginLayoutParams::copyMarginsFrom(const MarginLayoutParams& source){
    width = source.width;
    height = source.height;
    leftMargin = source.leftMargin;
    topMargin = source.topMargin;
    rightMargin = source.rightMargin;
    bottomMargin = source.bottomMargin;
    startMargin = source.startMargin;
    endMargin = source.endMargin;

    mMarginFlags = source.mMarginFlags;
}

void MarginLayoutParams::setMargins(int left, int top, int right, int bottom){
    leftMargin = left;
    topMargin = top;
    rightMargin = right;
    bottomMargin = bottom;
    mMarginFlags &= ~LEFT_MARGIN_UNDEFINED_MASK;
    mMarginFlags &= ~RIGHT_MARGIN_UNDEFINED_MASK;
    if (isMarginRelative()) {
        mMarginFlags |= NEED_RESOLUTION_MASK;
    } else {
        mMarginFlags &= ~NEED_RESOLUTION_MASK;
    }
}

void MarginLayoutParams::setMarginsRelative(int start, int top, int end, int bottom){
    startMargin = start;
    topMargin = top;
    endMargin = end;
    bottomMargin = bottom;
    mMarginFlags |= NEED_RESOLUTION_MASK;
}

void MarginLayoutParams::setMarginStart(int start){
    startMargin = start;
    mMarginFlags |= NEED_RESOLUTION_MASK;
}

int MarginLayoutParams::getMarginStart(){
    if (startMargin != DEFAULT_MARGIN_RELATIVE) return startMargin;
    if ((mMarginFlags & NEED_RESOLUTION_MASK) == NEED_RESOLUTION_MASK) {
       doResolveMargins();
    }
    switch(mMarginFlags & LAYOUT_DIRECTION_MASK) {
    case View::LAYOUT_DIRECTION_RTL:return rightMargin;
    case View::LAYOUT_DIRECTION_LTR:
    default:return leftMargin;
    }
}

void MarginLayoutParams::setMarginEnd(int end){
    endMargin = end;
    mMarginFlags |= NEED_RESOLUTION_MASK;
}

int MarginLayoutParams::getMarginEnd(){
    if (endMargin != DEFAULT_MARGIN_RELATIVE) return endMargin;
    if ((mMarginFlags & NEED_RESOLUTION_MASK) == NEED_RESOLUTION_MASK) {
        doResolveMargins();
    }
    switch(mMarginFlags & LAYOUT_DIRECTION_MASK) {
    case View::LAYOUT_DIRECTION_RTL:return leftMargin;
    case View::LAYOUT_DIRECTION_LTR:
    default:return rightMargin;
    }
}

bool MarginLayoutParams::isMarginRelative()const{
    return (startMargin != DEFAULT_MARGIN_RELATIVE || endMargin != DEFAULT_MARGIN_RELATIVE);
}

void MarginLayoutParams::setLayoutDirection(int layoutDirection){
    if (layoutDirection != View::LAYOUT_DIRECTION_LTR &&
          layoutDirection != View::LAYOUT_DIRECTION_RTL) return;
    if (layoutDirection != (mMarginFlags & LAYOUT_DIRECTION_MASK)) {
        mMarginFlags &= ~LAYOUT_DIRECTION_MASK;
        mMarginFlags |= (layoutDirection & LAYOUT_DIRECTION_MASK);
        if (isMarginRelative()) {
            mMarginFlags |= NEED_RESOLUTION_MASK;
        } else {
            mMarginFlags &= ~NEED_RESOLUTION_MASK;
        }
    }
}

int MarginLayoutParams::getLayoutDirection()const{
    return (mMarginFlags & LAYOUT_DIRECTION_MASK);
}

void MarginLayoutParams::resolveLayoutDirection(int layoutDirection){
    setLayoutDirection(layoutDirection);

    // No relative margin or pre JB-MR1 case or no need to resolve, just dont do anything
    // Will use the left and right margins if no relative margin is defined.
    if (!isMarginRelative() || (mMarginFlags & NEED_RESOLUTION_MASK) != NEED_RESOLUTION_MASK) return;

    // Proceed with resolution
    doResolveMargins();
}

bool MarginLayoutParams::isLayoutRtl()const{
    return ((mMarginFlags & LAYOUT_DIRECTION_MASK) == View::LAYOUT_DIRECTION_RTL);
}

static void fillRect(Canvas& canvas,int x1, int y1, int x2, int y2) {
    if (x1 == x2 || y1 == y2) return;
    if (x1 > x2) {
        int tmp = x1; x1 = x2; x2 = tmp;
    }
    if (y1 > y2) {
        int tmp = y1; y1 = y2; y2 = tmp;
    }
    canvas.rectangle(x1, y1, x2-x1, y2-y1);
}

static void fillDifference(Canvas& canvas, int x2, int y2, int x3, int y3,
        int dx1, int dy1, int dx2, int dy2) {
    int x1 = x2 - dx1;
    int y1 = y2 - dy1;
    int x4 = x3 + dx2;
    int y4 = y3 + dy2;

    fillRect(canvas, x1, y1, x4, y2);
    fillRect(canvas, x1, y2, x2, y3);
    fillRect(canvas, x3, y2, x4, y3);
    fillRect(canvas, x1, y3, x4, y4);
}

void MarginLayoutParams::onDebugDraw(View&view, Canvas&canvas){
    Insets oi = (view.getParent() && view.getParent()->isLayoutModeOptical()) ? view.getOpticalInsets() : Insets::NONE;
    fillDifference(canvas, view.getLeft() + oi.left,  view.getTop() + oi.top,
        view.getRight() - oi.right,  view.getBottom() - oi.bottom ,
        leftMargin, topMargin, rightMargin, bottomMargin);
}

void MarginLayoutParams::doResolveMargins(){
    if ((mMarginFlags & RTL_COMPATIBILITY_MODE_MASK) == RTL_COMPATIBILITY_MODE_MASK) {
        // if left or right margins are not defined and if we have some start or end margin
        // defined then use those start and end margins.
        if ((mMarginFlags & LEFT_MARGIN_UNDEFINED_MASK) == LEFT_MARGIN_UNDEFINED_MASK
               && startMargin > DEFAULT_MARGIN_RELATIVE) {
            leftMargin = startMargin;
        }
        if ((mMarginFlags & RIGHT_MARGIN_UNDEFINED_MASK) == RIGHT_MARGIN_UNDEFINED_MASK
               && endMargin > DEFAULT_MARGIN_RELATIVE) {
            rightMargin = endMargin;
        }
    } else {
        // We have some relative margins (either the start one or the end one or both). So use
        // them and override what has been defined for left and right margins. If either start
        // or end margin is not defined, just set it to default "0".
        switch(mMarginFlags & LAYOUT_DIRECTION_MASK) {
        case View::LAYOUT_DIRECTION_RTL:
             leftMargin = (endMargin > DEFAULT_MARGIN_RELATIVE) ?
                     endMargin : DEFAULT_MARGIN_RESOLVED;
             rightMargin = (startMargin > DEFAULT_MARGIN_RELATIVE) ?
                     startMargin : DEFAULT_MARGIN_RESOLVED;
             break;
        case View::LAYOUT_DIRECTION_LTR:
        default:
             leftMargin = (startMargin > DEFAULT_MARGIN_RELATIVE) ?
                     startMargin : DEFAULT_MARGIN_RESOLVED;
             rightMargin = (endMargin > DEFAULT_MARGIN_RELATIVE) ?
                     endMargin : DEFAULT_MARGIN_RESOLVED;
             break;
        }
    }
    mMarginFlags &= ~NEED_RESOLUTION_MASK;
}

}//namespace

