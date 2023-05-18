#include <widget/compoundbutton.h>
#include <cdlog.h>
namespace cdroid{

DECLARE_WIDGET(CompoundButton)

CompoundButton::CompoundButton(Context*ctx,const AttributeSet& attrs)
  :Button(ctx,attrs){
    initCompoundButton();
    setButtonDrawable(attrs.getString("button"));
    setChecked(attrs.getBoolean("checked"));
}

CompoundButton::CompoundButton(const std::string&txt,int width,int height)
    :Button(txt,width,height){
    initCompoundButton();
}

void CompoundButton::initCompoundButton(){
    mChecked=false;
    mBroadcasting=false;
    mCheckedFromResource=false;
    mButtonDrawable=nullptr;
    mOnCheckedChangeListener=nullptr;
    mOnCheckedChangeWidgetListener=nullptr;
    isChecked = [this]()->bool{
        return mChecked;
    };
    toggle = [this](){
        setChecked(!mChecked);
    };
    setChecked = std::bind(&CompoundButton::doSetChecked,this,std::placeholders::_1);
}

CompoundButton::~CompoundButton(){
    delete mButtonDrawable;
}

std::vector<int>CompoundButton::onCreateDrawableState(){
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

void CompoundButton::doSetChecked(bool checked){
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

int CompoundButton::getCompoundPaddingLeft() {
    int padding = Button::getCompoundPaddingLeft();
    if ((false==isLayoutRtl()) && mButtonDrawable) {
        padding += mButtonDrawable->getIntrinsicWidth();
    }
    return padding;
}

int CompoundButton::getCompoundPaddingRight() {
    int padding = Button::getCompoundPaddingRight();
    if (isLayoutRtl() && mButtonDrawable) {
        padding += mButtonDrawable->getIntrinsicWidth();
    }
    return padding;
}

int CompoundButton::getHorizontalOffsetForDrawables()const{
    return (mButtonDrawable == nullptr) ? 0 : mButtonDrawable->getIntrinsicWidth();
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

        const int left = isLayoutRtl() ? getWidth() - drawableWidth : 0;
        mButtonDrawable->setBounds(left,top, drawableWidth ,drawableHeight);
        Drawable* background = getBackground();
        if (background != nullptr) {
            background->setHotspotBounds(left, top,drawableWidth,drawableHeight);
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

