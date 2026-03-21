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
#include <widget/cardview.h>
#include <widget/cardviewimpl.h>
namespace cdroid{
class CardView::CardViewDelegateInternal:public CardViewDelegate{
private:
    CardView*mCardView;
    Drawable* mCardBackground;
public:
    CardViewDelegateInternal(CardView*cv):mCardView(cv){
        mCardBackground=nullptr;
    }
    void setCardBackground(Drawable* drawable) override{
        mCardBackground = drawable;
        mCardView->setBackground(drawable);
    }

    bool getUseCompatPadding() override{
        return mCardView->getUseCompatPadding();
    }

    bool getPreventCornerOverlap() override{
        return mCardView->getPreventCornerOverlap();
    }

    void setShadowPadding(int left, int top, int right, int bottom) override{
        mCardView->mShadowBounds.set(left, top, right, bottom);
        //CardView.super.setPadding(left + mContentPadding.left, top + mContentPadding.top,
        //        right + mContentPadding.right, bottom + mContentPadding.bottom);
        Rect& mContentPadding=mCardView->mContentPadding;
        dynamic_cast<FrameLayout*>(mCardView)->setPadding(left + mContentPadding.left, top + mContentPadding.top,
            right + mContentPadding.width, bottom + mContentPadding.height);
    }

    void setMinWidthHeightInternal(int width, int height) override{
        if (width > mCardView->mUserSetMinWidth) {
            //CardView.super.setMinimumWidth(width);
            dynamic_cast<FrameLayout*>(mCardView)->setMinimumWidth(width);
        }
        if (height > mCardView->mUserSetMinHeight) {
            //CardView.super.setMinimumHeight(height);
            dynamic_cast<FrameLayout*>(mCardView)->setMinimumHeight(height);
        }
    }

    Drawable* getCardBackground() override{
        return mCardBackground;
    }

    View* getCardView() override{
        return mCardView;
    }
};

static CardViewApi21Impl mCardViewApi21Impl;
static CardViewBaseImpl mCardViewBaseImpl;
CardViewImpl* CardView::IMPL = &mCardViewBaseImpl;//&mCardViewApi21Impl;

DECLARE_WIDGET(CardView);

CardView::CardView(int w,int h):FrameLayout(w,h){
    mCardViewDelegate = new CardViewDelegateInternal(this);
    mCompatPadding = false;
    mPreventCornerOverlap =true;
    mUserSetMinWidth =0;
    mUserSetMinHeight=0;
    IMPL->initialize(mCardViewDelegate, mContext, 0, 0, 0, 0);
}

CardView::CardView(Context* context, const AttributeSet& attrs)
    :FrameLayout(context, attrs){
    mCardViewDelegate = new CardViewDelegateInternal(this);
    //TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.CardView, defStyleAttr,R.style.CardView);
    cdroid::RefPtr<ColorStateList> backgroundColor;
    if (attrs.hasAttribute("cardBackgroundColor")) {
        backgroundColor = attrs.getColorStateList("cardBackgroundColor");
    } /*else {
        // There isn't one set, so we'll compute one based on the theme
        final TypedArray aa = getContext().obtainStyledAttributes(COLOR_BACKGROUND_ATTR);//R.attr.colorBackground
        final int themeColorBackground = aa.getColor(0, 0);
        aa.recycle();

        // If the theme colorBackground is light, use our own light color, otherwise dark
        final float[] hsv = new float[3];
        Color.colorToHSV(themeColorBackground, hsv);
        backgroundColor = ColorStateList.valueOf(hsv[2] > 0.5f
                ? getResources().getColor(R.color.cardview_light_background)
                : getResources().getColor(R.color.cardview_dark_background));
    }*/
    const float radius = attrs.getDimension("cardCornerRadius", 0);
    const float elevation = attrs.getDimension("cardElevation", 0);
    float maxElevation = attrs.getDimension("cardMaxElevation", 0);
    mCompatPadding = attrs.getBoolean("cardUseCompatPadding", false);
    mPreventCornerOverlap = attrs.getBoolean("cardPreventCornerOverlap", true);
    const int defaultPadding = attrs.getDimensionPixelSize("contentPadding", 0);
    mContentPadding.left = attrs.getDimensionPixelSize("contentPaddingLeft", defaultPadding);
    mContentPadding.top = attrs.getDimensionPixelSize("contentPaddingTop", defaultPadding);
    mContentPadding.width = attrs.getDimensionPixelSize("contentPaddingRight", defaultPadding);
    mContentPadding.height = attrs.getDimensionPixelSize("contentPaddingBottom", defaultPadding);
    if (elevation > maxElevation) {
        maxElevation = elevation;
    }
    mUserSetMinWidth = attrs.getDimensionPixelSize("minWidth", 0);
    mUserSetMinHeight = attrs.getDimensionPixelSize("minHeight", 0);

    IMPL->initialize(mCardViewDelegate, context, backgroundColor, radius, elevation, maxElevation);
}

CardView::~CardView(){
    delete mCardViewDelegate;
}

void CardView::setPadding(int left, int top, int right, int bottom) {
    // NO OP
}

void CardView::setPaddingRelative(int start, int top, int end, int bottom) {
    // NO OP
}

bool CardView::getUseCompatPadding() const{
    return mCompatPadding;
}

void CardView::setUseCompatPadding(bool useCompatPadding) {
    if (mCompatPadding != useCompatPadding) {
        mCompatPadding = useCompatPadding;
        IMPL->onCompatPaddingChanged(mCardViewDelegate);
    }
}

void CardView::setContentPadding(int left, int top, int right, int bottom) {
    mContentPadding.set(left, top, right, bottom);
    IMPL->updatePadding(mCardViewDelegate);
}

void CardView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    if (dynamic_cast<CardViewApi21Impl*>(IMPL)!=nullptr) {
        int minWidth,minHeight;
        const int widthMode = MeasureSpec::getMode(widthMeasureSpec);
        switch (widthMode) {
        case MeasureSpec::EXACTLY:
        case MeasureSpec::AT_MOST:
            minWidth = (int) std::ceil(IMPL->getMinWidth(mCardViewDelegate));
            widthMeasureSpec = MeasureSpec::makeMeasureSpec(std::max(minWidth,
                    MeasureSpec::getSize(widthMeasureSpec)), widthMode);
            break;
        case MeasureSpec::UNSPECIFIED:
            // Do nothing
            break;
        }

        const int heightMode = MeasureSpec::getMode(heightMeasureSpec);
        switch (heightMode) {
        case MeasureSpec::EXACTLY:
        case MeasureSpec::AT_MOST:
            minHeight = (int) std::ceil(IMPL->getMinHeight(mCardViewDelegate));
            heightMeasureSpec = MeasureSpec::makeMeasureSpec(std::max(minHeight,
                    MeasureSpec::getSize(heightMeasureSpec)), heightMode);
            break;
        case MeasureSpec::UNSPECIFIED:
            // Do nothing
            break;
        }
        FrameLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
    } else {
        FrameLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
    }
}

void CardView::setMinimumWidth(int minWidth) {
    mUserSetMinWidth = minWidth;
    FrameLayout::setMinimumWidth(minWidth);
}

void CardView::setMinimumHeight(int minHeight) {
    mUserSetMinHeight = minHeight;
    FrameLayout::setMinimumHeight(minHeight);
}

void CardView::setCardBackgroundColor(int color) {
    IMPL->setBackgroundColor(mCardViewDelegate, ColorStateList::valueOf(color));
}

void CardView::setCardBackgroundColor(const cdroid::RefPtr<ColorStateList>& color) {
    IMPL->setBackgroundColor(mCardViewDelegate, color);
}

const cdroid::RefPtr<ColorStateList> CardView::getCardBackgroundColor() const{
    return IMPL->getBackgroundColor(mCardViewDelegate);
}

int CardView::getContentPaddingLeft() const{
    return mContentPadding.left;
}

int CardView::getContentPaddingRight() const{
    return mContentPadding.width;
}

int CardView::getContentPaddingTop() const{
    return mContentPadding.top;
}

int CardView::getContentPaddingBottom() const{
    return mContentPadding.height;
}

void CardView::setRadius(float radius) {
    IMPL->setRadius(mCardViewDelegate, radius);
}

float CardView::getRadius() const{
    return IMPL->getRadius(mCardViewDelegate);
}

void CardView::setElevation(float elevation){
    IMPL->setElevation(mCardViewDelegate, elevation);
}

float CardView::getElevation()const {
    return IMPL->getElevation(mCardViewDelegate);
}

void CardView::setCardElevation(float elevation) {
    IMPL->setElevation(mCardViewDelegate, elevation);
}

float CardView::getCardElevation() const{
    return IMPL->getElevation(mCardViewDelegate);
}

void CardView::setMaxCardElevation(float maxElevation) {
    IMPL->setMaxElevation(mCardViewDelegate, maxElevation);
}

float CardView::getMaxCardElevation() const{
    return IMPL->getMaxElevation(mCardViewDelegate);
}

bool CardView::getPreventCornerOverlap() const{
    return mPreventCornerOverlap;
}

void CardView::setPreventCornerOverlap(bool preventCornerOverlap) {
    if (preventCornerOverlap != mPreventCornerOverlap) {
        mPreventCornerOverlap = preventCornerOverlap;
        IMPL->onPreventCornerOverlapChanged(mCardViewDelegate);
    }
}
}/*endof namespace*/
