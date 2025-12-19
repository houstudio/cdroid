#include <widget/checkedtextview.h>
namespace cdroid{

DECLARE_WIDGET(CheckedTextView)

CheckedTextView::CheckedTextView(Context* context,const AttributeSet& a):TextView(context,a){
    Drawable* d = context->getDrawable(a.getString("checkMark"));
    mCheckMarkDrawable = nullptr;
    mCheckMarkTintList = nullptr;
    mHasCheckMarkTintMode=false;
    mHasCheckMarkTint = false;
    if (d)setCheckMarkDrawable(d);

    if (a.hasAttribute("checkMarkTintMode")) {
        //mCheckMarkBlendMode = Drawable.parseBlendMode(a.getInt(
        //   R.styleable.CheckedTextView_checkMarkTintMode, -1), mCheckMarkBlendMode);
        mHasCheckMarkTintMode = true;
    }

    if (a.hasAttribute("checkMarkTint")) {
        mCheckMarkTintList = a.getColorStateList("checkMarkTint");
        mHasCheckMarkTint = (mCheckMarkTintList!=nullptr);
    }
    mChecked = false;
    mCheckMarkGravity = a.getGravity("checkMarkGravity", Gravity::END);

    const bool checked = a.getBoolean("checked", false);
    setChecked(checked);
    applyCheckMarkTint();
#if defined(FUNCTION_AS_CHECKABLE)&&FUNCTION_AS_CHECKABLE
    isChecked = [this]()->bool{
        return mChecked;
    };
    toggle = [this](){
        doSetChecked(!mChecked);
    };
    setChecked = [this](bool checked){
        doSetChecked(checked);
   };
#endif
}

CheckedTextView::~CheckedTextView(){
    delete mCheckMarkDrawable;
    delete mCheckMarkTintList;
}

#if !(defined(FUNCTION_AS_CHECKABLE)&&FUNCTION_AS_CHECKABLE)
void CheckedTextView::toggle() {
    setChecked(!mChecked);
}

bool CheckedTextView::isChecked()const{
   return mChecked;
}

void CheckedTextView::setChecked(bool checked) {
    doSetChecked(checked);
}
#endif
void CheckedTextView::doSetChecked(bool checked) {
    if (mChecked != checked) {
        mChecked = checked;
        refreshDrawableState();
        notifyViewAccessibilityStateChangedIfNeeded(AccessibilityEvent::CONTENT_CHANGE_TYPE_UNDEFINED);
    }
}

void CheckedTextView::setCheckMarkDrawable(const std::string&resId) {
    if (resId.empty()==false && resId == mCheckMarkResource) {
        return;
    }

    Drawable* d = resId.empty()==false ? getContext()->getDrawable(resId) : nullptr;
    setCheckMarkDrawableInternal(d, resId);
}

void CheckedTextView::setCheckMarkDrawable(Drawable* d) {
    setCheckMarkDrawableInternal(d, "");
}

void CheckedTextView::setCheckMarkDrawableInternal(Drawable* d,const std::string&resId){
    if (mCheckMarkDrawable) {
        mCheckMarkDrawable->setCallback(nullptr);
        unscheduleDrawable(*mCheckMarkDrawable);
    }

    mNeedRequestlayout = (d != mCheckMarkDrawable);

    if (d != nullptr) {
        d->setCallback(this);
        d->setVisible(getVisibility() == VISIBLE, false);
        d->setState(StateSet::get(StateSet::VIEW_STATE_CHECKED));

        // Record the intrinsic dimensions when in "checked" state.
        setMinHeight(d->getIntrinsicHeight());
        mCheckMarkWidth = d->getIntrinsicWidth();

        d->setState(getDrawableState());
    } else {
        mCheckMarkWidth = 0;
    }

    mCheckMarkDrawable = d;
    mCheckMarkResource = resId;

    applyCheckMarkTint();

    // Do padding resolution. This will call internalSetPadding() and do a
    // requestLayout() if needed.
    resolvePadding();
}

void CheckedTextView::setCheckMarkTintList(const ColorStateList*tint){
    if(mCheckMarkTintList!=tint){
        mCheckMarkTintList = tint;
        mHasCheckMarkTint = (tint!=nullptr);
        applyCheckMarkTint();
    }
}

const ColorStateList* CheckedTextView::getCheckMarkTintList()const{
    return mCheckMarkTintList;
}

void CheckedTextView::applyCheckMarkTint() {
    if (mCheckMarkDrawable && (mHasCheckMarkTint || mHasCheckMarkTintMode)) {
        mCheckMarkDrawable = mCheckMarkDrawable->mutate();

        if (mHasCheckMarkTint) {
            mCheckMarkDrawable->setTintList(mCheckMarkTintList);
        }

        //if (mHasCheckMarkTintMode) mCheckMarkDrawable->setTintBlendMode(mCheckMarkBlendMode);

        // The drawable (or one of its children) may not have been
        // stateful before applying the tint, so let's try again.
        if (mCheckMarkDrawable->isStateful()) {
            mCheckMarkDrawable->setState(getDrawableState());
        }
    }
}

void CheckedTextView::setVisibility(int visibility) {
    TextView::setVisibility(visibility);

    if (mCheckMarkDrawable) {
        mCheckMarkDrawable->setVisible(visibility == VISIBLE, false);
    }
}

void CheckedTextView::jumpDrawablesToCurrentState() {
    TextView::jumpDrawablesToCurrentState();

    if (mCheckMarkDrawable) {
        mCheckMarkDrawable->jumpToCurrentState();
    }
}

bool CheckedTextView::verifyDrawable(Drawable* who)const{
    return who == mCheckMarkDrawable || TextView::verifyDrawable(who);
}

Drawable* CheckedTextView::getCheckMarkDrawable()const{
    return mCheckMarkDrawable;
}

void CheckedTextView::internalSetPadding(int left, int top, int right, int bottom) {
    TextView::internalSetPadding(left, top, right, bottom);
    setBasePadding(isCheckMarkAtStart());
}

void CheckedTextView::onRtlPropertiesChanged(int layoutDirection) {
    TextView::onRtlPropertiesChanged(layoutDirection);
    updatePadding();
}

void CheckedTextView::updatePadding() {
    resetPaddingToInitialValues();
    const int newPadding = (mCheckMarkDrawable) ?
         mCheckMarkWidth + mBasePadding : mBasePadding;
    if (isCheckMarkAtStart()) {
        mNeedRequestlayout |= (mPaddingLeft != newPadding);
        mPaddingLeft = newPadding;
    } else {
        mNeedRequestlayout |= (mPaddingRight != newPadding);
        mPaddingRight = newPadding;
    }
    if (mNeedRequestlayout) {
        requestLayout();
        mNeedRequestlayout = false;
    }
}

void CheckedTextView::setBasePadding(bool checkmarkAtStart) {
    if (checkmarkAtStart) {
        mBasePadding = mPaddingLeft;
    } else {
        mBasePadding = mPaddingRight;
    }
}

bool CheckedTextView::isCheckMarkAtStart() {
    const int gravity = Gravity::getAbsoluteGravity(mCheckMarkGravity, getLayoutDirection());
    const int hgrav = gravity & Gravity::HORIZONTAL_GRAVITY_MASK;
    return hgrav == Gravity::LEFT;
}

void CheckedTextView::onDetachedFromWindowInternal(){
    TextView::onDetachedFromWindowInternal();
    if(mCheckMarkDrawable)
        unscheduleDrawable(*mCheckMarkDrawable);
}

void CheckedTextView::onDraw(Canvas& canvas) {
    TextView::onDraw(canvas);

    Drawable* checkMarkDrawable = mCheckMarkDrawable;
    if (checkMarkDrawable != nullptr) {
       const int verticalGravity = getGravity() & Gravity::VERTICAL_GRAVITY_MASK;
       const int height = checkMarkDrawable->getIntrinsicHeight();

       int y = 0;

       switch (verticalGravity) {
       case Gravity::BOTTOM:
            y = getHeight() - height;
            break;
       case Gravity::CENTER_VERTICAL:
            y = (getHeight() - height) / 2;
            break;
       }

       const bool checkMarkAtStart = isCheckMarkAtStart();
       const int width = getWidth();
       const int top = y;
       int left,right;
       if (checkMarkAtStart) {
           left = mBasePadding;
           //right = left + mCheckMarkWidth;
       } else {
           //right = width - mBasePadding;
           left = width - mBasePadding - mCheckMarkWidth;
       }
       checkMarkDrawable->setBounds(mScrollX + left, top, mCheckMarkWidth, height);
       checkMarkDrawable->draw(canvas);

       Drawable* background = getBackground();
       if (background != nullptr) {
           background->setHotspotBounds(mScrollX + left, top, mCheckMarkWidth, height);
       }
    }
}

std::vector<int> CheckedTextView::onCreateDrawableState(int extraSpace){
    std::vector<int> drawableState = TextView::onCreateDrawableState(extraSpace);
    if (isChecked()) {
        mergeDrawableStates(drawableState, StateSet::get(StateSet::VIEW_STATE_CHECKED));
    }
    return drawableState;
}

void CheckedTextView::drawableStateChanged() {
    TextView::drawableStateChanged();

    Drawable* checkMarkDrawable = mCheckMarkDrawable;
    if (checkMarkDrawable && checkMarkDrawable->isStateful()
            && checkMarkDrawable->setState(getDrawableState())) {
        invalidateDrawable(*checkMarkDrawable);
    }
}

void CheckedTextView::drawableHotspotChanged(float x, float y) {
    TextView::drawableHotspotChanged(x, y);

    if (mCheckMarkDrawable) {
        mCheckMarkDrawable->setHotspot(x, y);
    }
}

std::string CheckedTextView::getAccessibilityClassName()const{
    return "CheckedTextView";
}

void CheckedTextView::onInitializeAccessibilityEventInternal(AccessibilityEvent& event) {
    TextView::onInitializeAccessibilityEventInternal(event);
    event.setChecked(mChecked);
}

void CheckedTextView::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info) {
    TextView::onInitializeAccessibilityNodeInfoInternal(info);
    info.setCheckable(true);
    info.setChecked(mChecked);
}

}
