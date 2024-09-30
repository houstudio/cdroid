#include <widgetEx/qrcodeview.h>
#if ENABLE(QRCODE)
#include <qrencode.h>
#include <cdlog.h>

//REF:https://github.com/zint/zint-gpl-only/blob/master/backend_qt4/qzint.h/cpp

namespace cdroid{

DECLARE_WIDGET(QRCodeView)

QRCodeView::QRCodeView(int w,int h):View(w,h){
    initView();
};

QRCodeView::QRCodeView(Context*ctx,const AttributeSet&attrs):View(ctx,attrs){
    initView();
    mEccLevel = attrs.getInt("eccLevel",std::map<const std::string,int>{
            {"low",QR_ECLEVEL_L},
            {"medium",QR_ECLEVEL_M},
            {"quartor",QR_ECLEVEL_Q},
            {"high",QR_ECLEVEL_H}
    },mEccLevel);
    mDotColor =attrs.getColor("dotColor",mDotColor);
    mLogoDrawable = attrs.getDrawable("logo");
}

QRCodeView::~QRCodeView(){
    delete mLogoDrawable;
}

void QRCodeView::initView(){
    mZoom = 1.0;
    mQrCodeWidth =0;
    mEccLevel = QR_ECLEVEL_M;
    mMode = QR_MODE_8;
    mDotColor=0xFF000000;
    mLogoDrawable = nullptr;
    setBackgroundColor(0xFF000000);
}

void QRCodeView::setDotColor(int color){
    if(mDotColor!=color){
        mDotColor=color;
        invalidate();
    }
}

int QRCodeView::getDotColor()const{
    return mDotColor;
}

void QRCodeView::setText(const std::string&text){
    if(mText!=text){
        if(mText.empty())
            requestLayout();
        mText = text;
        encode();
        invalidate(true);
    }
    float w,h;
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
    if(mQRImage)
        mZoom = float(std::min(w,h))/float(mQRImage->get_width());
}

void  QRCodeView::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    const int widthMode  = MeasureSpec::getMode(widthMeasureSpec);
    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    const int widthSize  = MeasureSpec::getSize(widthMeasureSpec);
    const int heightSize = MeasureSpec::getSize(heightMeasureSpec);
    int width = widthSize,height=heightSize;
    switch(widthMode){
    case MeasureSpec::EXACTLY:
        height= width; break;
    case MeasureSpec::AT_MOST:
        if(heightMode==MeasureSpec::EXACTLY){
            height= heightSize;
        }else
            width = mQrCodeWidth*mZoom;
        break;
    case MeasureSpec::UNSPECIFIED:
        break;
    }
    mZoom = std::min(width,height);
    mZoom /= mQrCodeWidth;
    LOGV("setMeasuredDimension(%d,%d)  %dx%d mZoom=%f",width,height,widthSize,heightSize,mZoom);
    setMeasuredDimension(width, height);
}

extern "C" int QRspec_getMinimumVersion(int size, QRecLevel level);

void QRCodeView::encode(){
    const int version = QRspec_getMinimumVersion(mText.size(), (QRecLevel)mEccLevel);
    QRcode*mQRcode = QRcode_encodeString(mText.c_str(), version, (QRecLevel)mEccLevel, (QRencodeMode)mMode, 1);
    if(mQRcode){
        const uint8_t*qrd = mQRcode->data;
        mQrCodeWidth = mQRcode->width;
        mQRImage = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,mQrCodeWidth,mQrCodeWidth);
        const uint32_t image_stride = mQRImage->get_stride()/4;
        uint32_t*qimg = (uint32_t*)mQRImage->get_data();
        const int barBgColor = (~mDotColor)|0xFF000000;
        for(int32_t y = 0,idx = 0; y < mQRcode->width; y++){
            for(int32_t x = 0; x < mQRcode->width; x++){
                qimg[x]=(qrd[idx+x]&1)?mDotColor:barBgColor;
            }
            idx += mQRcode->width;
            qimg += image_stride;
        }
        const float wx = getWidth() - getPaddingLeft()- getPaddingRight();
        const float wy = getHeight()- getPaddingTop() - getPaddingBottom();
        mZoom = std::min(wx,wy)/mQRcode->width;
        mQRImage->mark_dirty();
    }
    QRcode_free(mQRcode);
}

void  QRCodeView::onDraw(Canvas&canvas){
    View::onDraw(canvas);

    canvas.save();

    canvas.translate(getPaddingLeft(), getPaddingTop());
    canvas.scale(mZoom,mZoom);
    canvas.set_source(mQRImage,0,0);
    Cairo::RefPtr<Cairo::SurfacePattern>spat = canvas.get_source_for_surface();
    spat->set_filter(Cairo::SurfacePattern::Filter::NEAREST);
    canvas.rectangle(0,0,mQrCodeWidth,mQrCodeWidth);
    canvas.clip();
    canvas.paint();
    canvas.restore();

    if(mLogoDrawable&&(mEccLevel>=QR_ECLEVEL_M)){
        Rect rect;
        const static float ff[]={0.5,0.3,0.25,0.2};
        const int imgw = mLogoDrawable->getIntrinsicWidth();
        const int imgh = mLogoDrawable->getIntrinsicHeight();
        rect.set(getPaddingLeft(),getPaddingTop(),
                getWidth()-getPaddingLeft()-getPaddingRight(),
                getHeight()-getPaddingTop()-getPaddingBottom());
        rect.inflate(-getWidth()*ff[mEccLevel],-getHeight()*ff[mEccLevel]);
        if(imgw*imgh<rect.width*rect.height)
            rect.set((getWidth()-imgw)/2,(getHeight()-imgh)/2,imgw,imgh);
        mLogoDrawable->setBounds(rect);
        mLogoDrawable->draw(canvas);
    }
}

}/*endof namespace*/

#endif/*ENABLE(BARCODE)*/

