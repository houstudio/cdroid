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
#ifndef __QRCODE_VIEW_H__
#define __QRCODE_VIEW_H__

#include <view/view.h>

namespace cdroid{
class QRCodeView:public View{
public:
    enum ECCLevel{
        ECC_LOW=0,
        ECC_MEDIUM=1,
        ECC_QUARTOR=2,
        ECC_HIGH=3
    };
    enum QREncodeMode{
        MODE_NUMERIC=0,
        MODE_ALPHANUMERIC=1,
        MODE_UTF8=2,
        MODE_KANJI=3,
        MODE_ECI=4
    };
private:
    int mQrCodeWidth;
    int mDotColor;
    int mBarBgColor;
    int mEccLevel;
    int mEncodeMode;
    float mZoom;
    bool mShowLogo;
    Drawable*mLogoDrawable;
private:
    void initView();
protected:
    std::string mText;
    Cairo::RefPtr<Cairo::ImageSurface>mQRImage;
    void encode();
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onSizeChanged(int w,int h,int ow,int oh)override;
    bool verifyDrawable(Drawable* who)const override;
    void jumpDrawablesToCurrentState()override;
    void onResolveDrawables(int layoutDirection)override;
    bool onTouchEvent(MotionEvent&)override;
public:
    QRCodeView(int w,int h);
    QRCodeView(Context*ctx,const AttributeSet&attrs);
    ~QRCodeView()override;
    void setText(const std::string&text);
    void setEccLevel(int);
    int getEccLevel()const;
    void setEncodeMode(int);
    int getEncodeMode()const;
    void setDotColor(int color);
    int getDotColor()const;
    void setBarBgColor(int color);
    int getBarBgColor()const;
    void setLogo(Drawable*);
    void setLogoResource(const std::string&);
    Drawable* getLogo()const;
    void setZoom(float);
    float getZoom()const;
    void onDraw(Canvas&canvas)override;
public:

};
}/*endof namespace*/
#endif/*__BARCODE_VIEW_H__*/

