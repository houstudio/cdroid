#ifndef __QRRCODE_VIEW_H__
#define __QRCODE_VIEW_H__

#include <view/view.h>

namespace cdroid{
class QRCodeView:public View{
private:
    void initView();
    bool resetSymbol();
protected:
    float mZoom;
    std::string mText;
    Cairo::RefPtr<Cairo::ImageSurface>mQRImage;
    void encode();
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
public:
    QRCodeView(int w,int h);
    QRCodeView(Context*ctx,const AttributeSet&attrs);
    ~QRCodeView()override;
    void setText(const std::string&text);
    void setBarcodeColor(int color);
    int  getBarcodeColor()const;
    void setZoom(float);
    float getZoom()const;
    void onDraw(Canvas&canvas)override;
public:

};
}/*endof namespace*/
#endif/*__BARCODE_VIEW_H__*/

