#include <cdroid.h>
#if ENABLE_BARCODE
#include <ext_widget/barcodeview.h>
#endif

int main(int argc,const char*argv[]){
#if ENABLE_BARCODE
     App app(argc,argv);
     Window*w =new Window(0,0,-1,-1); 
     BarcodeView*bv=new BarcodeView(200,200);
     w->addView(bv);
     bv->setText("https://www.sina.com.cn");
     app.exec();
#endif
}
