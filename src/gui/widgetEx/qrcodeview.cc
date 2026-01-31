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
#include <widgetEx/qrcodeview.h>
#if ENABLE(QRCODE)
#include <widgetEx/qrcodegen.h>
#include <float.h>
#include <cdlog.h>

//REF:https://github.com/zint/zint-gpl-only/blob/master/backend_qt4/qzint.h/cpp

namespace cdroid{

DECLARE_WIDGET(QRCodeView)

QRCodeView::QRCodeView(int w,int h):View(w,h){
    initView();
    encode();
};

QRCodeView::QRCodeView(Context*ctx,const AttributeSet&attrs):View(ctx,attrs){
    initView();

    mEccLevel = attrs.getInt("eccLevel",std::unordered_map<std::string,int>{
            {"low",ECC_LOW},    /* 7%*/
            {"medium",ECC_MEDIUM}, /*15%*/
            {"quartor",ECC_QUARTOR},/*20%*/
            {"high",ECC_HIGH}    /*30%*/
    },mEccLevel);

    mEncodeMode = attrs.getInt("encodeMode",std::unordered_map<std::string,int>{
            {"numberic",MODE_NUMERIC},
            {"alphanumeric",MODE_ALPHANUMERIC},
            {"utf8" , MODE_UTF8},
            {"kanji", MODE_KANJI}
    },mEncodeMode);

    mDotColor  = attrs.getColor("dotColor",mDotColor);
    mBarBgColor= attrs.getColor("barBgColor", (~mDotColor)|0xFF000000);
    mLogoDrawable = attrs.getDrawable("logo");
    encode();
}

QRCodeView::~QRCodeView(){
    delete mLogoDrawable;
}

void QRCodeView::initView(){
    mZoom = 1.0;
    mQrCodeWidth =0;
    mEccLevel = ECC_MEDIUM;
    mEncodeMode = MODE_UTF8;
    mDotColor = 0xFF000000;
    mBarBgColor= (~mDotColor)|0xFF000000;
    mShowLogo = true;
    mLogoDrawable = nullptr;
}

void QRCodeView::setEccLevel(int level){
    if(mEccLevel!=level){
        mEccLevel=level;
        encode();
        invalidate();
    }
}

int QRCodeView::getEccLevel()const{
    return mEccLevel;
}

void QRCodeView::setEncodeMode(int mode){
    if((mEncodeMode!=mode)&&(mode>=MODE_NUMERIC)&&(mode<=MODE_KANJI)){
        mEncodeMode = mode;
        encode();
        invalidate();
    }
}

int QRCodeView::getEncodeMode()const{
    return mEncodeMode;
}

void QRCodeView::setDotColor(int color){
    if(mDotColor!=color){
        mDotColor=color;
        encode();
        invalidate();
    }
}

int QRCodeView::getDotColor()const{
    return mDotColor;
}

void QRCodeView::setBarBgColor(int color){
    if(mBarBgColor!=color){
        mBarBgColor = color;
        encode( );
        invalidate();
    }
}

int QRCodeView::getBarBgColor()const{
    return mBarBgColor;
}

void QRCodeView::setText(const std::string&text){
    if(mText!=text){
        mText = text;
        encode();
        requestLayout();
        invalidate(true);
    }
}

void QRCodeView::setLogoResource(const std::string&resid){
    setLogo(mContext->getDrawable(resid));
}

void QRCodeView::setLogo(Drawable*logo){
    if(mLogoDrawable!=logo){
        delete mLogoDrawable;
        mLogoDrawable = logo;
        invalidate();
    }
}

Drawable* QRCodeView::getLogo()const{
    return mLogoDrawable;
}

bool QRCodeView::verifyDrawable(Drawable* who)const{
    return (who==mLogoDrawable)||View::verifyDrawable(who);
}

void QRCodeView::jumpDrawablesToCurrentState(){
    View::jumpDrawablesToCurrentState();
    if (mLogoDrawable) mLogoDrawable->jumpToCurrentState();
}

void QRCodeView::onResolveDrawables(int layoutDirection){
    if(mLogoDrawable)
        mLogoDrawable->setLayoutDirection(layoutDirection);
}

void  QRCodeView::setZoom(float zoom){
    mZoom = zoom;
    requestLayout();
}

float  QRCodeView::getZoom()const{
    return mZoom;
}

void QRCodeView::onSizeChanged(int w,int h,int ow,int oh){
    View::onSizeChanged(w,h,ow,oh);
    if(mQRImage&&mQrCodeWidth>0){
        mZoom = float(std::min(w-getPaddingLeft()- getPaddingRight(),
                    h-getPaddingTop() - getPaddingBottom()))/mQrCodeWidth;
    }
}

void  QRCodeView::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    const int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    const int widthSize = MeasureSpec::getSize(widthMeasureSpec);
    const int heightSize = MeasureSpec::getSize(heightMeasureSpec);

    int desiredWidth = mQrCodeWidth;
    int desiredHeight = mQrCodeWidth;

    int measuredWidth = 0;
    int measuredHeight = 0;

    switch (widthMode) {
    case MeasureSpec::EXACTLY:
        measuredWidth = widthSize;
        break;
    case MeasureSpec::AT_MOST:
        measuredWidth = std::min(desiredWidth, widthSize);
        break;
    case MeasureSpec::UNSPECIFIED:
        measuredWidth = desiredWidth;
        break;
    }

    switch (heightMode) {
    case MeasureSpec::EXACTLY:
        measuredHeight = heightSize;
        break;
    case MeasureSpec::AT_MOST:
        measuredHeight = std::min(desiredHeight, heightSize);
        break;
    case MeasureSpec::UNSPECIFIED:
        measuredHeight = desiredHeight;
        break;
    }

    const int availableWidth = measuredWidth - getPaddingLeft() - getPaddingRight();
    const int availableHeight = measuredHeight - getPaddingTop() - getPaddingBottom();

    if (mQrCodeWidth > 0 && availableWidth > 0 && availableHeight > 0) {
        const float scaleToFitWidth = static_cast<float>(availableWidth) / mQrCodeWidth;
        const float scaleToFitHeight = static_cast<float>(availableHeight) / mQrCodeWidth;

        mZoom = std::min(scaleToFitWidth, scaleToFitHeight);

        const int actualContentWidth = static_cast<int>(mQrCodeWidth * mZoom);
        const int actualContentHeight = static_cast<int>(mQrCodeWidth * mZoom); // 正方形

        measuredWidth = actualContentWidth + getPaddingLeft() + getPaddingRight();
        measuredHeight = actualContentHeight + getPaddingTop() + getPaddingBottom();
    } else if (mQrCodeWidth <= 0) {
        // 如果 mQrCodeWidth 无效，可能需要设置一个最小默认值或返回
        // 或者，如果允许 UNSPECIFIED 情况，可以保持 desiredWidth/desiredHeight
        // 这里暂时不做特殊处理，使用前面根据 mode 计算出的值
    }

    if (widthMode == MeasureSpec::AT_MOST) {
        measuredWidth = std::min(measuredWidth, widthSize);
    }
    if (heightMode == MeasureSpec::AT_MOST) {
        measuredHeight = std::min(measuredHeight, heightSize);
    }
    setMeasuredDimension(measuredWidth, measuredHeight);
}

void QRCodeView::encode(){
    const float wx = getWidth() - getPaddingLeft()- getPaddingRight();
    const float wy = getHeight()- getPaddingTop() - getPaddingBottom();
    qrcodegen::QrCode qr0 = qrcodegen::QrCode::encodeText(mText.c_str(),static_cast<qrcodegen::QrCode::Ecc>(mEccLevel));
    mQrCodeWidth = qr0.getSize();
    mZoom = std::min(wx,wy)/mQrCodeWidth;
    mQRImage = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,mQrCodeWidth,mQrCodeWidth);

    const uint32_t image_stride = mQRImage->get_stride()/4;
    uint32_t*qimg = (uint32_t*)mQRImage->get_data();
    for(int32_t y = 0,idx = 0; y < mQrCodeWidth; y++){
        for(int32_t x = 0; x < mQrCodeWidth; x++){
            qimg[x] = qr0.getModule(x, y)?mDotColor:mBarBgColor;
        }
        idx += mQrCodeWidth;
        qimg += image_stride;
    }
    mQRImage->mark_dirty();
}

bool QRCodeView::onTouchEvent(MotionEvent&evt){
    if(evt.getActionMasked()==MotionEvent::ACTION_UP){
        mShowLogo = !mShowLogo;
        invalidate();
    }
    return View::onTouchEvent(evt);
}

void  QRCodeView::onDraw(Canvas&canvas){
    View::onDraw(canvas);

    if((mZoom<=FLT_EPSILON)||std::isfinite(mZoom)){
        LOGW("mZoom=%f",mZoom);
        return;
    }
    canvas.save();

    canvas.translate(getPaddingLeft(), getPaddingTop());
    canvas.scale(mZoom,mZoom);
    canvas.set_source(mQRImage,0,0);
    auto pat = canvas.get_source_for_surface();
    pat->set_filter(Cairo::SurfacePattern::Filter::NEAREST);
    canvas.rectangle(0,0,mQrCodeWidth,mQrCodeWidth);
    canvas.clip();
    canvas.paint();
    canvas.restore();

    if(mLogoDrawable&&mShowLogo){
        Rect rect;
        const static float ff[]={/*0.07,0.15,0.20,0.30*/
            0.05,0.12,0.16,0.256};
        const int dec = (std::sqrt(mQrCodeWidth*mQrCodeWidth*ff[mEccLevel]))*mZoom;
        const int imgw = mLogoDrawable->getIntrinsicWidth();
        const int imgh = mLogoDrawable->getIntrinsicHeight();
        rect.set(getPaddingLeft(),getPaddingTop(),
                getWidth()-getPaddingLeft()-getPaddingRight(),
                getHeight()-getPaddingTop()-getPaddingBottom());
        LOGV("level=%d mQrCodeWidth=%d dec=%d zoom=%f imgsize=%dx%d",mEccLevel,mQrCodeWidth,dec,mZoom,imgw,imgh);
        if(imgw*imgh>dec*dec)
            rect.set((getWidth()-dec)/2,(getHeight()-dec)/2,dec,dec);
        else
            rect.set((getWidth()-imgw)/2,(getHeight()-imgh)/2,imgw,imgh);
        mLogoDrawable->setBounds(rect);
        mLogoDrawable->draw(canvas);
    }
}

}/*endof namespace*/

#endif/*ENABLE(BARCODE)*/

