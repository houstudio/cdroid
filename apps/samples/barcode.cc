#include <cdroid.h>
#if ENABLE_BARCODE
#include <widgetEx/barcodeview.h>
#include <widgetEx/qrcodeview.h>
#endif

int main(int argc,const char*argv[]){
     App app(argc,argv);
     Window*w =new Window(0,0,-1,-1); 
#if ENABLE(BARCODE)
     LinearLayout*ll=new LinearLayout(800,512);
     ll->setOrientation(LinearLayout::VERTICAL);
     BarcodeView*bv=new BarcodeView(256,256);
     ll->addView(bv,new LinearLayout::LayoutParams(-2,256));
     bv->setBackgroundColor(0xFFDDEEFF);
     bv->setPadding(10,10,10,10);
     bv->setZoom(2.f);
     bv->setSymbology(BarcodeView::QRCode);
     bv->setText("https://www.sina.com.cn");
     BarcodeView*bv2=new BarcodeView(500,160);
     bv2->setSymbology(BarcodeView::Code128);
     ll->addView(bv2,new LinearLayout::LayoutParams(-2,-2));
     bv2->setText("138-0123456789");
#endif
#if ENABLE(QRCODE)
     QRCodeView *qr=new QRCodeView(256,256);
     qr->setPadding(10,10,10,10);
     qr->setText("138-0123456789");
     ll->addView(qr,new LinearLayout::LayoutParams(-2,256));
#endif
     w->addView(ll);
     ll->requestLayout();
     w->setBackgroundColor(0xff112233);
     app.exec();
}
