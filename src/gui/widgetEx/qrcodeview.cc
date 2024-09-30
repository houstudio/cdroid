#include <widgetEx/qrcodeview.h>
#if ENABLE(QRCODE)
#include <widgetEx/qrcodegen.h>
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
            {"low",QR_ECLEVEL_L},    /* 7%*/
            {"medium",QR_ECLEVEL_M}, /*15%*/
            {"quartor",QR_ECLEVEL_Q},/*20%*/
            {"high",QR_ECLEVEL_H}    /*30%*/
    },mEccLevel);

    mEncodeMode = attrs.getInt("encodeMode",std::map<const std::string,int>{
            {"numberic",int(MODE_NUMERIC)},
            {"alphanumeric",int(MODE_ALPHANUMERIC)},
            {"utf8",int(MODE_UTF8)},
            {"kanji",int(MODE_KANJI)}
    },mEncodeMode);

    mDotColor = attrs.getColor("dotColor",mDotColor);
    mLogoDrawable = attrs.getDrawable("logo");
}

QRCodeView::~QRCodeView(){
    delete mLogoDrawable;
}

void QRCodeView::initView(){
    mZoom = 1.0;
    mQrCodeWidth =0;
    mEccLevel = QR_ECLEVEL_H;
    mEncodeMode = QR_MODE_8;
    mDotColor=0xFF000000;
    mLogoDrawable = nullptr;
    setBackgroundColor(0xFF000000);
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
    const float wx = getWidth() - getPaddingLeft()- getPaddingRight();
    const float wy = getHeight()- getPaddingTop() - getPaddingBottom();
    qrcodegen::QrCode qr0 = qrcodegen::QrCode::encodeText(mText.c_str(),static_cast<qrcodegen::QrCode::Ecc>(mEccLevel));
    //std::vector<QrSegment> segs= QrSegment::makeSegments(mText);
    mQrCodeWidth = qr0.getSize();
    mZoom = std::min(wx,wy)/mQrCodeWidth;
    mQRImage = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,mQrCodeWidth,mQrCodeWidth);

    const uint32_t image_stride = mQRImage->get_stride()/4;
    uint32_t*qimg = (uint32_t*)mQRImage->get_data();
    mDotColor = 0xFF000000;
    const int barBgColor = (~mDotColor)|0xFF000000;
    for(int32_t y = 0,idx = 0; y < mQrCodeWidth; y++){
        for(int32_t x = 0; x < mQrCodeWidth; x++){
            qimg[x] = qr0.getModule(x, y)?mDotColor:barBgColor;
        }
        idx += mQrCodeWidth;
        qimg += image_stride;
    }
    mQRImage->mark_dirty();
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

    if(mLogoDrawable){
        Rect rect;
        const static float ff[]={/*0.07,0.15,0.20,0.30*/
            0.05,0.12,0.18,0.28};
        const int dec = (std::sqrt(mQrCodeWidth*mQrCodeWidth*ff[mEccLevel]))*mZoom;
        const int imgw = mLogoDrawable->getIntrinsicWidth();
        const int imgh = mLogoDrawable->getIntrinsicHeight();
        rect.set(getPaddingLeft(),getPaddingTop(),
                getWidth()-getPaddingLeft()-getPaddingRight(),
                getHeight()-getPaddingTop()-getPaddingBottom());
        LOGD("level=%d mQrCodeWidth=%d dec=%d zoom=%f imgsize=%dx%d",mEccLevel,mQrCodeWidth,dec,mZoom,imgw,imgh);
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

