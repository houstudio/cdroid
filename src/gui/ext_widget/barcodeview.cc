#if ENABLE_BARCODE
#include <zint.h>
#include <string.h>
#include <ext_widget/barcodeview.h>
#include <cdlog.h>

namespace cdroid{

BarcodeView::BarcodeView(int w,int h):View(w,h){
    initView();
};

BarcodeView::BarcodeView(Context*ctx,const AttributeSet&attrs):View(ctx,attrs){
    initView();

}

BarcodeView::~BarcodeView(){
    ZBarcode_Clear(mSymbol);
    ZBarcode_Delete(mSymbol);
}

void BarcodeView::initView(){
    mSymbol = ZBarcode_Create();
    mSymbol->outfile[0]=0;
    mSymbol->symbology = QRCode;
    mSymbol->whitespace_width =2;
    mSymbol->whitespace_height=2;
    mSymbol->bitmap_width=200;
    mSymbol->bitmap_height=200;
    mSymbol->debug=1;
    mSymbol->scale=4.0;
}

int BarcodeView::getSymbology()const{
    return mSymbol->symbology;
}

void BarcodeView::setSymbology(int code){
    const int rc=ZBarcode_ValidID(code);
    LOGE_IF(rc,"%d is not an valid Symbologies",code);
    if(rc==0){
        mSymbol->symbology = code;
        if(!mText.empty()) invalidate();
    }
}

std::string BarcodeView::getBarcodeName(){
    char name[32];
    ZBarcode_BarcodeName(mSymbol->symbology,name);
    return std::string(name);
}

void BarcodeView::setText(const std::string&text){
    mText = text;
    ZBarcode_Encode_and_Print(mSymbol,(const unsigned char*)mText.c_str(),mText.length(),45);
    LOGD("xdim=%f",ZBarcode_Default_Xdim(mSymbol->symbology));
}

}/*endof namespace*/

#endif/*ENABLE_BARCODE*/

