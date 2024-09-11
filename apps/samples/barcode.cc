#include <cdroid.h>
#if ENABLE_BARCODE
#include <widgetEx/barcodeview.h>
#include <widgetEx/qrcodeview.h>
#endif

int main(int argc,const char*argv[]){
     App app(argc,argv);
     Window*w =new Window(0,0,-1,-1); 
#if ENABLE(BARCODE)
     BarcodeView*bv=new BarcodeView(256,256);
     w->addView(bv);
     bv->setZoom(2.f);
     bv->setSymbology(BarcodeView::QRCode);
     bv->setText("https://www.sina.com.cn");
     BarcodeView*bv2=new BarcodeView(500,160);
     bv2->setSymbology(BarcodeView::Code11);
     w->addView(bv2).setPos(0,300);
     bv2->setText("138-0123456789");
#endif
#if ENABLE(QRCODE)
     QRCodeView *qr=new QRCodeView(256,256);
     qr->setText("138-0123456789");
     w->addView(qr).setPos(300,0);
#endif
     w->setBackgroundColor(0xff112233);
     app.exec();
}
