#ifndef __QRCODE_VIEW_H__
#define __QRCODE_VIEW_H__

#include <view/view.h>

namespace cdroid{
class QRCodeView:public View{
private:
    int mQrCodeWidth;
    int mDotColor;
    int mEccLevel;
    int mMode;
    float mZoom;
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
public:
    QRCodeView(int w,int h);
    QRCodeView(Context*ctx,const AttributeSet&attrs);
    ~QRCodeView()override;
    void setDotColor(int color);
    int getDotColor()const;
    void setText(const std::string&text);
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

