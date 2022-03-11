#include <widget/compoundbutton.h>
#include <cdlog.h>
namespace cdroid{

DECLARE_WIDGET(CompoundButton)

CompoundButton::CompoundButton(Context*ctx,const AttributeSet& attrs)
  :Button(ctx,attrs){
    mButtonDrawable=nullptr;
    mOnCheckedChangeListener=nullptr;
    mOnCheckedChangeWidgetListener=nullptr;
    setButtonDrawable(attrs.getString("button"));
    setChecked(attrs.getBoolean("checked"));
}

CompoundButton::CompoundButton(const std::string&txt,int width,int height)
    :Button(txt,width,height){
    mChecked=false;
    mBroadcasting=false;
    mCheckedFromResource=false;
    mButtonDrawable=nullptr;
    mOnCheckedChangeListener=nullptr;
    mOnCheckedChangeWidgetListener=nullptr;
}

std::vector<int>CompoundButton::onCreateDrawableState()const{
    std::vector<int>drawableState = Button::onCreateDrawableState();
    if (isChecked()) {
        mergeDrawableStates(drawableState,StateSet::get(StateSet::VIEW_STATE_CHECKED));
    }
    return drawableState;
}

void CompoundButton::drawableStateChanged() {
    Button::drawableStateChanged();
    if (mButtonDrawable  && mButtonDrawable->isStateful()
            && mButtonDrawable->setState(getDrawableState())) {
        invalidateDrawable(*mButtonDrawable);
    }
}

void CompoundButton::drawableHotspotChanged(float x,float y){
    Button::drawableHotspotChanged(x,y);
    if(mButtonDrawable)mButtonDrawable->setHotspot(x,y);
}

int CompoundButton::getHorizontalOffsetForDrawables()const{
    return (mButtonDrawable == nullptr) ?0: mButtonDrawable->getIntrinsicWidth();
}

void CompoundButton::toggle(){
    setChecked(!mChecked);
}

bool CompoundButton::performClick(){
    toggle();
    const bool handled = Button::performClick();
    if (!handled) {
        // View only makes a sound effect if the onClickListener was
        // called, so we'll need to make one here instead.
        //playSoundEffect(SoundEffectConstants.CLICK);
    }
    return handled;
}

void CompoundButton::setButtonDrawable(const std::string&resid){
    Drawable* d= getContext()->getDrawable(resid);
    setButtonDrawable(d);
}

void CompoundButton::setButtonDrawable(Drawable*drawable){
    if (mButtonDrawable != drawable) {
        if (mButtonDrawable != nullptr) {
            mButtonDrawable->setCallback(nullptr);
            unscheduleDrawable(*mButtonDrawable);
        }
        delete mButtonDrawable;
        mButtonDrawable = drawable;

        if (drawable != nullptr) {
            drawable->setCallback(this);
            drawable->setLayoutDirection(getLayoutDirection());
            if (drawable->isStateful()) drawable->setState(getDrawableState());

            drawable->setVisible(getVisibility() == VISIBLE, false);
            setMinHeight(drawable->getIntrinsicHeight());
            //applyButtonTint();
        }
    } 
}

Drawable* CompoundButton::getButtonDrawable()const{
    return mButtonDrawable;
}

bool CompoundButton::verifyDrawable(Drawable* who)const{
    return Button::verifyDrawable(who) || who == mButtonDrawable;
}

void CompoundButton::jumpDrawablesToCurrentState(){
    Button::jumpDrawablesToCurrentState();
    if (mButtonDrawable ) mButtonDrawable->jumpToCurrentState();
}

bool CompoundButton::isChecked()const{
    return mChecked;
}

void CompoundButton::setChecked(bool checked){
    if (mChecked != checked) {
        mCheckedFromResource = false;
        mChecked = checked;
        refreshDrawableState();
        //notifyViewAccessibilityStateChangedIfNeeded(AccessibilityEvent.CONTENT_CHANGE_TYPE_UNDEFINED);
        // Avoid infinite recursions if setChecked() is called from a listener
        if (mBroadcasting)return;

        mBroadcasting = true;
        if (mOnCheckedChangeListener) mOnCheckedChangeListener(*this, mChecked);
        if (mOnCheckedChangeWidgetListener ) mOnCheckedChangeWidgetListener(*this, mChecked);
        //final AutofillManager afm = mContext.getSystemService(AutofillManager.class);
        //if (afm != null)  afm.notifyValueChanged(this);
        
        mBroadcasting = false;
    }    
}

void CompoundButton::setOnCheckedChangeListener(OnCheckedChangeListener listener) {
    mOnCheckedChangeListener = listener;
}

void CompoundButton::setOnCheckedChangeWidgetListener(OnCheckedChangeListener listener) {
    mOnCheckedChangeWidgetListener = listener;
}

void CompoundButton::onDraw(Canvas&canvas){
    if (mButtonDrawable != nullptr) {
        const int verticalGravity = getGravity() & Gravity::VERTICAL_GRAVITY_MASK;
        const int drawableHeight = mButtonDrawable->getIntrinsicHeight();
        const int drawableWidth = mButtonDrawable->getIntrinsicWidth();

        int top;
        switch (verticalGravity) {
        case Gravity::BOTTOM         : top = getHeight() - drawableHeight;         break;
        case Gravity::CENTER_VERTICAL: top = (getHeight() - drawableHeight) / 2;   break;
        default:           top = 0;
        }
        const int bottom = top + drawableHeight;
        const int left = isLayoutRtl() ? getWidth() - drawableWidth : 0;
        const int right = isLayoutRtl() ? getWidth() : drawableWidth;

        mButtonDrawable->setBounds(left, top, drawableWidth,drawableHeight);//right, bottom);
        Drawable* background = getBackground();
        if (background != nullptr) {
            //background->setHotspotBounds(left, top, right, bottom);
        }
    }

    Button::onDraw(canvas);
    if (mButtonDrawable != nullptr) {
        if (mScrollX == 0 && mScrollY == 0) {
            mButtonDrawable->draw(canvas);
        } else {
            canvas.translate(mScrollX, mScrollY);
            mButtonDrawable->draw(canvas);
            canvas.translate(-mScrollX, -mScrollY);
        }
    }
}

}

