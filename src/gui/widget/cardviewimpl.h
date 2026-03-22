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
#ifndef __CARDVIEW_IMPL_H__
#define __CARDVIEW_IMPL_H__
#include <core/context.h>
#include <drawable/roundrectdrawable.h>
#include <drawable/roundrectdrawablewithshadow.h>
namespace cdroid{
class CardViewDelegate {
public:
    virtual ~CardViewDelegate()=default;
    virtual void setCardBackground(Drawable* drawable)=0;
    virtual Drawable* getCardBackground()=0;
    virtual bool getUseCompatPadding()=0;
    virtual bool getPreventCornerOverlap()=0;
    virtual void setShadowPadding(int left, int top, int right, int bottom)=0;
    virtual void setMinWidthHeightInternal(int width, int height)=0;
    virtual View* getCardView()=0;
};

class CardViewImpl {
public:
    virtual ~CardViewImpl()=default;
    virtual void initialize(CardViewDelegate* cardView, Context* context, const cdroid::RefPtr<ColorStateList>& backgroundColor,
            float radius, float elevation, float maxElevation)=0;

    virtual void setRadius(CardViewDelegate* cardView, float radius)=0;
    virtual float getRadius(CardViewDelegate* cardView)=0;

    virtual void setElevation(CardViewDelegate* cardView, float elevation)=0;
    virtual float getElevation(CardViewDelegate* cardView)=0;

    virtual void initStatic()=0;

    virtual void setMaxElevation(CardViewDelegate* cardView, float maxElevation)=0;
    virtual float getMaxElevation(CardViewDelegate* cardView)=0;

    virtual float getMinWidth(CardViewDelegate* cardView)=0;
    virtual float getMinHeight(CardViewDelegate* cardView)=0;

    virtual void updatePadding(CardViewDelegate* cardView)=0;

    virtual void onCompatPaddingChanged(CardViewDelegate* cardView)=0;

    virtual void onPreventCornerOverlapChanged(CardViewDelegate* cardView)=0;

    virtual void setBackgroundColor(CardViewDelegate* cardView,const cdroid::RefPtr<ColorStateList>& color)=0;
    virtual const cdroid::RefPtr<ColorStateList> getBackgroundColor(CardViewDelegate* cardView)=0;
};

class CardViewApi21Impl:public CardViewImpl {
private:
    RoundRectDrawable* getCardBackground(CardViewDelegate* cardView){
        return ((RoundRectDrawable*) cardView->getCardBackground());
    }
public:
    void initialize(CardViewDelegate* cardView, Context* context, const cdroid::RefPtr<ColorStateList>& backgroundColor,
            float radius, float elevation, float maxElevation) override{
        RoundRectDrawable* background = new RoundRectDrawable(backgroundColor, radius);
        cardView->setCardBackground(background);

        View* view = cardView->getCardView();
        view->setClipToOutline(true);
        view->setElevation(elevation);
        setMaxElevation(cardView, maxElevation);
    }

    void setRadius(CardViewDelegate* cardView, float radius) override{
        getCardBackground(cardView)->setRadius(radius);
    }

    void initStatic() override{
    }

    void setMaxElevation(CardViewDelegate* cardView, float maxElevation) override{
        getCardBackground(cardView)->setPadding(maxElevation,
                cardView->getUseCompatPadding(), cardView->getPreventCornerOverlap());
        updatePadding(cardView);
    }

    float getMaxElevation(CardViewDelegate* cardView) override{
        return getCardBackground(cardView)->getPadding();
    }

    float getMinWidth(CardViewDelegate* cardView) override{
        return getRadius(cardView) * 2;
    }

    float getMinHeight(CardViewDelegate* cardView) override{
        return getRadius(cardView) * 2;
    }

    float getRadius(CardViewDelegate* cardView) override{
        return getCardBackground(cardView)->getRadius();
    }

    void setElevation(CardViewDelegate* cardView, float elevation) override{
        cardView->getCardView()->setElevation(elevation);
    }

    float getElevation(CardViewDelegate* cardView) override{
        return cardView->getCardView()->getElevation();
    }

    void updatePadding(CardViewDelegate* cardView) override{
        if (!cardView->getUseCompatPadding()) {
            cardView->setShadowPadding(0, 0, 0, 0);
            return;
        }
        const float elevation = getMaxElevation(cardView);
        const float radius = getRadius(cardView);
        const int hPadding = (int) std::ceil(RoundRectDrawableWithShadow
                ::calculateHorizontalPadding(elevation, radius, cardView->getPreventCornerOverlap()));
        const int vPadding = (int) std::ceil(RoundRectDrawableWithShadow
                ::calculateVerticalPadding(elevation, radius, cardView->getPreventCornerOverlap()));
        cardView->setShadowPadding(hPadding, vPadding, hPadding, vPadding);
    }

    void onCompatPaddingChanged(CardViewDelegate* cardView) override{
        setMaxElevation(cardView, getMaxElevation(cardView));
    }

    void onPreventCornerOverlapChanged(CardViewDelegate* cardView) override{
        setMaxElevation(cardView, getMaxElevation(cardView));
    }

    void setBackgroundColor(CardViewDelegate* cardView,const cdroid::RefPtr<ColorStateList>& color) override{
        getCardBackground(cardView)->setColor(color);
    }

    const cdroid::RefPtr<ColorStateList> getBackgroundColor(CardViewDelegate* cardView) override{
        return getCardBackground(cardView)->getColor();
    }
};

class CardViewBaseImpl:public CardViewImpl {
private:
    RoundRectDrawableWithShadow* createBackground(Context* context, const cdroid::RefPtr<ColorStateList>& backgroundColor,
            float radius, float elevation, float maxElevation) {
        return new RoundRectDrawableWithShadow(context, backgroundColor, radius, elevation, maxElevation);
    }
    RoundRectDrawableWithShadow* getShadowBackground(CardViewDelegate* cardView) {
        return ((RoundRectDrawableWithShadow*) cardView->getCardBackground());
    }
public:
    void initStatic() override{
        /*RoundRectDrawableWithShadow::sRoundRectHelper =
                (canvas, bounds, cornerRadius, paint) ->
                        canvas.drawRoundRect(bounds, cornerRadius, cornerRadius, paint);*/
    }

    void initialize(CardViewDelegate* cardView, Context* context, const cdroid::RefPtr<ColorStateList>& backgroundColor,
            float radius, float elevation, float maxElevation) override{
        RoundRectDrawableWithShadow* background = createBackground(context, backgroundColor, radius,
                elevation, maxElevation);
        background->setAddPaddingForCorners(cardView->getPreventCornerOverlap());
        cardView->setCardBackground(background);
        updatePadding(cardView);
    }


    void updatePadding(CardViewDelegate* cardView) override{
        Rect shadowPadding;
        getShadowBackground(cardView)->getMaxShadowAndCornerPadding(shadowPadding);
        cardView->setMinWidthHeightInternal((int) std::ceil(getMinWidth(cardView)),
                (int) std::ceil(getMinHeight(cardView)));
        cardView->setShadowPadding(shadowPadding.left, shadowPadding.top,
                shadowPadding.width, shadowPadding.height);
    }

    void onCompatPaddingChanged(CardViewDelegate* cardView) override{
        // NO OP
    }

    void onPreventCornerOverlapChanged(CardViewDelegate* cardView) override{
        getShadowBackground(cardView)->setAddPaddingForCorners(cardView->getPreventCornerOverlap());
        updatePadding(cardView);
    }

    void setBackgroundColor(CardViewDelegate* cardView, const cdroid::RefPtr<ColorStateList>& color) override{
        getShadowBackground(cardView)->setColor(color);
    }

    const cdroid::RefPtr<ColorStateList> getBackgroundColor(CardViewDelegate* cardView) override{
        return getShadowBackground(cardView)->getColor();
    }

    void setRadius(CardViewDelegate* cardView, float radius) override{
        getShadowBackground(cardView)->setCornerRadius(radius);
        updatePadding(cardView);
    }

    float getRadius(CardViewDelegate* cardView) override{
        return getShadowBackground(cardView)->getCornerRadius();
    }

    void setElevation(CardViewDelegate* cardView, float elevation) override{
        getShadowBackground(cardView)->setShadowSize(elevation);
    }

    float getElevation(CardViewDelegate* cardView) override{
        return getShadowBackground(cardView)->getShadowSize();
    }

    void setMaxElevation(CardViewDelegate* cardView, float maxElevation) override{
        getShadowBackground(cardView)->setMaxShadowSize(maxElevation);
        updatePadding(cardView);
    }

    float getMaxElevation(CardViewDelegate* cardView) override{
        return getShadowBackground(cardView)->getMaxShadowSize();
    }

    float getMinWidth(CardViewDelegate* cardView) override{
        return getShadowBackground(cardView)->getMinWidth();
    }

    float getMinHeight(CardViewDelegate* cardView) {
        return getShadowBackground(cardView)->getMinHeight();
    }
};
}/*endof namespace*/
#endif/*__CARDVIEW_IMPL_H__*/
