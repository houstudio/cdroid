#pragma once

#if ENABLE_BARCODE

#include <view/view.h>
#include <string>
#include <zint.h>
struct zint_symbol *symbol;

namespace cdroid{
class BarcodeView:public View{
public:
    enum BorderType{NO_BORDER=0, TOP=1 , BIND=2, BOX=4};
    enum AspectRatioMode{IgnoreAspectRatio=0, KeepAspectRatio=1, CenterBarCode=2};
    class ZintSeg {
    public:
	std::string mText;//`seg->source` and `seg->length`
        int mECI; //`seg->eci`
        ZintSeg();
        ZintSeg(const std::string& text, const int ECIIndex = 0); // `ECIIndex` is comboBox index (not ECI value)
    };
    enum Symbologies{
	Code11   = BARCODE_CODE11,      /*1 Code 11 */ 
	C25Standard=BARCODE_C25STANDARD,/*2 2 of 5 Standard (Matrix) */
        C25Matrix= BARCODE_C25MATRIX,   /*2 Legacy */
	C25Inter = BARCODE_C25INTER,    /*3 2 of 5 Interleaved */
	C25Data  = BARCODE_C25IATA,     /*4 2 of 5 IATA */
	C25Logic = BARCODE_C25LOGIC,    /*6 2 of 5 Data Logic */
	C25Ind   = BARCODE_C25IND,      /*7 2 of 5 Industrial */
	Code39   = BARCODE_CODE39,      /*8 Code 39 */
	ExCode39 = BARCODE_EXCODE39,    /*9 Extended Code 39 */
	Eanx     = BARCODE_EANX,        /*13 EAN (European Article Number) */
	EanxChk  = BARCODE_EANX_CHK,    /*14 EAN + Check Digit */
	GS1128   = BARCODE_GS1_128,     /*16 GS1-128 */
	Ean128   = BARCODE_EAN128,      /*16 Legacy */
	CodaBar  = BARCODE_CODABAR,     /*18 Codabar */
	Code128  = BARCODE_CODE128,     /*20 Code 128 */
	DPLeit   = BARCODE_DPLEIT,      /*21 Deutsche Post Leitcode */
        DPIdent  = BARCODE_DPIDENT,     /*22 Deutsche Post Identcode */
        Code16K  = BARCODE_CODE16K,     /*23 Code 16k */
        Code49   = BARCODE_CODE49,      /*24 Code 49 */
        Code93   = BARCODE_CODE93,      /*25 Code 93 */
        Flat     = BARCODE_FLAT ,       /*28 Flattermarken */
        DBAR_OMN = BARCODE_DBAR_OMN,    /*29 GS1 DataBar Omnidirectional */
        RSS14    = BARCODE_RSS14,       /*29 Legacy */
        DBAR_LTD = BARCODE_DBAR_LTD,    /*30 GS1 DataBar Limited */
        RSS_LTD  = BARCODE_RSS_LTD,     /*30 Legacy */
        DBAR_EXP = BARCODE_DBAR_EXP,    /*31 GS1 DataBar Expanded */
        RSSEXP   = BARCODE_RSS_EXP,     /*31 Legacy */
        Telepen  = BARCODE_TELEPEN,     /*32 Telepen Alpha */
        UPCA     = BARCODE_UPCA,        /*34  UPC-A */
        UPCACHK  = BARCODE_UPCA_CHK,    /*35  UPC-A + Check Digit */
        UPCE     = BARCODE_UPCE,        /*37  UPC-E */
        UPCECHK  = BARCODE_UPCE_CHK,    /*38  UPC-E + Check Digit */
        PostNet  = BARCODE_POSTNET,     /* 40 USPS (U.S. Postal Service) POSTNET */
        MsiPlessey=BARCODE_MSI_PLESSEY, /*47 MSI Plessey */
        FIM      = BARCODE_FIM    ,     /*49 Facing Identification Mark */
        LOGMARS  = BARCODE_LOGMARS,     /*50 LOGMARS */
        PHARMA   = BARCODE_PHARMA,      /*51 Pharmacode One-Track */
        PZN      = BARCODE_PZN ,        /*52 Pharmazentralnummer */
        PharmaTwo= BARCODE_PHARMA_TWO,  /*53 Pharmacode Two-Track */
        CEPNet   = BARCODE_CEPNET    ,  /*54 razilian CEPNet Postal Code */
        PDF417   = BARCODE_PDF417    ,  /*55  PDF417 */
        PDF417Comp=BARCODE_PDF417COMP,  /*56  Compact PDF417 (Truncated PDF417) */
        PDF417Trunc=BARCODE_PDF417TRUNC,/*56 Legacy */
        MaxiCode = BARCODE_MAXICODE ,   /*57 MaxiCode */
        QRCode   = BARCODE_QRCODE   ,   /*58 QR Code */
        Code128AB= BARCODE_CODE128AB,   /*60 Code 128 (Suppress subset C) */
        Code128B = BARCODE_CODE128B,    /*60 Legacy */
        AusPost  = BARCODE_AUSPOST ,    /*63 Australia Post Standard Customer */
        AusReply = BARCODE_AUSREPLY,    /*66 Australia Post Reply Paid */
        AusRoute = BARCODE_AUSROUTE,    /*67 Australia Post Routing */
        AusRedirect=BARCODE_AUSREDIRECT,/*68 Australia Post Redirection */
        ISBNX    = BARCODE_ISBNX   ,    /*69 ISBN */
        RM4SCC   = BARCODE_RM4SCC  ,    /*70 Royal Mail 4-State Customer Code */
        DataMatrix=BARCODE_DATAMATRIX,  /*71 Data Matrix (ECC200) */
        EAN14    = BARCODE_EAN14    ,   /*72 EAN-14 */
        VIN      = BARCODE_VIN      ,   /*73 Vehicle Identification Number */
        CodaBlockF=BARCODE_CODABLOCKF,  /*74 Codablock-F */
        NVE18    = BARCODE_NVE18     ,  /*75 NVE-18 (SSCC-18) */
        JapanPost= BARCODE_JAPANPOST ,  /*76 Japanese Postal Code */
        KoreaPost= BARCODE_KOREAPOST ,  /*77 Korea Post */
        DBAR_STK = BARCODE_DBAR_STK   , /*79 GS1 DataBar Stacked */
        RSS14STACK=BARCODE_RSS14STACK,  /*79 Legacy */
        DBAR_OMNSTK=BARCODE_DBAR_OMNSTK,/*80 GS1 DataBar Stacked Omnidirectional */
        RSS14StackOmni=BARCODE_RSS14STACK_OMNI, /*80  Legacy */
        DBAR_EXPSTK= BARCODE_DBAR_EXPSTK, /*81  GS1 DataBar Expanded Stacked */
        RssExpStack= BARCODE_RSS_EXPSTACK,/*81  Legacy */
        PLANET   = BARCODE_PLANET      ,  /*82  USPS PLANET */
        MicroPDF417=BARCODE_MICROPDF417,  /*84  MicroPDF417 */
        USPS_IMail= BARCODE_USPS_IMAIL,   /*85  USPS Intelligent Mail (OneCode) */
        OneCode  = BARCODE_ONECODE,       /*85  Legacy */
        Plessy   = BARCODE_PLESSEY,       /*86  UK Plessey */

        /*barcode 8 codes */
        TelepenNum= BARCODE_TELEPEN_NUM,  /*87  Telepen Numeric */
        ITF14    = BARCODE_ITF14,         /*89  ITF-14 */
        KIX      = BARCODE_KIX  ,         /*90  Dutch Post KIX Code */
        Axtec    = BARCODE_AZTEC,         /*92  Aztec Code */
        DAFT     = BARCODE_DAFT,          /*93  DAFT Code */
        DPD      = BARCODE_DPD ,          /*96  DPD Code */
        MicroQR  = BARCODE_MICROQR,       /*97  Micro QR Code */

        /*barcode 9 codes */
        HIBC128  = BARCODE_HIBC_128,      /*98  HIBC (Health Industry Barcode) Code 128 */
        HIBC39   = BARCODE_HIBC_39,       /*99  HIBC Code 39 */
        HIBCDM   = BARCODE_HIBC_DM,       /*102 HIBC Data Matrix */
        HIBCQR   = BARCODE_HIBC_QR,       /*104 HIBC QR Code */
        HIBCPDF  = BARCODE_HIBC_PDF,      /*106 HIBC PDF417 */
        HIBCMicroPDF=BARCODE_HIBC_MICPDF, /*108 HIBC MicroPDF417 */
        HIBCBlockF=BARCODE_HIBC_BLOCKF,   /*110 HIBC Codablock-F */
        HIBCAztec= BARCODE_HIBC_AZTEC,    /*112 HIBC Aztec Code */

        /*barcode 10 codes */
        DotCode = BARCODE_DOTCODE,        /*115 DotCode */
        HanXin  = BARCODE_HANXIN,         /*116 Han Xin (Chinese Sensible) Code */

        /*barcode 11 codes */
        MailMark2D= BARCODE_MAILMARK_2D,  /*119 Royal Mail 2D Mailmark (CMDM) (Data Matrix) */
        UPU_S10  = BARCODE_UPU_S10,       /*120 Universal Postal Union S10 */
        MailMark4S=BARCODE_MAILMARK_4S,   /*121 Royal Mail 4-State Mailmark */
        MailMark = BARCODE_MAILMARK,      /*121 Legacy */

        /*int specific */
        AZRune = BARCODE_AZRUNE ,         /*128 Aztec Runes */
        Code32 = BARCODE_CODE32 ,         /*129 Code 32 */
        EAN_CC = BARCODE_EANX_CC,         /*130 EAN Composite */
        GS1_128_CC  = BARCODE_GS1_128_CC ,     /*131 GS1-128 Composite */
        EAN128_CC   = BARCODE_EAN128_CC  ,     /*131 Legacy */
        DBAR_OMN_CC = BARCODE_DBAR_OMN_CC,     /*132 GS1 DataBar Omnidirectional Composite */
        RSS14_CC    = BARCODE_RSS14_CC   ,     /*132 Legacy */
        DBAR_LTD_CC = BARCODE_DBAR_LTD_CC,     /*133 GS1 DataBar Limited Composite */
        DBAR_EXP_CC = BARCODE_DBAR_EXP_CC,     /*134 GS1 DataBar Expanded Composite */
        RSS_EXP_CC  = BARCODE_RSS_EXP_CC,      /*134 Legacy */
        UPCA_CC      = BARCODE_UPCA_CC,        /*135 UPC-A Composite */
        UPCE_CC      = BARCODE_UPCE_CC,        /*136 UPC-E Composite */
        DBAR_STK_CC  = BARCODE_DBAR_STK_CC,    /*137 GS1 DataBar Stacked Composite */
        RSS14STACK_CC= BARCODE_RSS14STACK_CC,  /*137 Legacy */
        OMNSTK_CC    = BARCODE_DBAR_OMNSTK_CC, /*138 GS1 DataBar Stacked Omnidirectional Composite */
        RSS14_OMNI_CC= BARCODE_RSS14_OMNI_CC,  /*138 Legacy */
        EXPSTK_CC    = BARCODE_DBAR_EXPSTK_CC, /*139 GS1 DataBar Expanded Stacked Composite */
        RSS_EXPSTACK_CC=BARCODE_RSS_EXPSTACK_CC, /*139 Legacy */
        Channel= BARCODE_CHANNEL ,         /*140 Channel Code */
        CodeOne= BARCODE_CODEONE ,         /*141 Code One */
        GridMatrix = BARCODE_GRIDMATRIX,   /*142 Grid Matrix */
        UPNQR = BARCODE_UPNQR,             /*143 UPNQR (Univerzalnega Plačilnega Naloga QR) */
        ULTRA = BARCODE_ULTRA,             /*144 Ultracode */
        RMQR  = BARCODE_RMQR ,             /*145 Rectangular Micro QR Code (rMQR) */
        BC412 = BARCODE_BC412,             /*146 IBM BC412 (SEMI T1-95) */
        LAST  = BARCODE_LAST               /*146Max barcode number marker, not barcode */
    };
private:
    int mErrorNo;
    int mRotateAngle;
    int mFgColor;
    int mSymbology;
    bool mDotty;
    bool mShowHRT;
    bool mCmyk;
    bool mGssep;
    bool mQuietZones;
    bool mNoQuietZones;
    bool mCompliantHeight;
    bool mReaderInit;
    bool mDebug;
    bool mGS1Parens;
    bool mGS1NoCheck;
    int mBorderType;
    int mOption1;
    int mOption2;
    int mOption3;
    int mDotSize;
    int mBorderWidth;
    int mWhiteSpace;
    int mVWhiteSpace;
    int mWarnLevel;
    int mECI;
    int mInputMode;
    int mEncodedRows;
    int mEncodedWidth;
    int mEncodedHeight;
    int mChanged;
    float mZoom;
    float mDpmm;
    float mGuardDescent;
    float mVectorWidth;
    float mVectorHeight;
    std::string mErrorStr;
    std::string mPrimaryMessage;
    std::vector<ZintSeg>mSegs;
    void initView();
    int convertSegs(struct zint_seg* zsegs, std::vector<std::string>& bstrs);
    bool resetSymbol();
protected:
    struct zint_symbol *mSymbol;
    std::string mText;
    void encode();
    /*Test capabilities*/
    bool hasHRT()const;
    bool isStackable()const;
    bool isExtendable()const;
    bool isComposite()const;
    bool supportsECI()const;
    bool supportsGS1()const;
    bool isDotty()const;
    bool hasDefaultQuietZones()const;
    bool isFixedRatio()const;
    bool supportsReaderInit()const;
    bool supportsFullMultibyte()const;
    bool hasMask()const;
    bool supportsStructApp()const;
    bool hasCompliantHeight()const;
    bool getWidthHeightXdim(float x_dim, float &width_x_dim, float &height_x_dim) const;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
public:
    BarcodeView(int w,int h);
    BarcodeView(Context*ctx,const AttributeSet&attrs);
    ~BarcodeView()override;
    void setText(const std::string&text);
    std::vector<ZintSeg> getSegs()const;
    void setSegs(const std::vector<ZintSeg>& segs);
    void setBarcodeColor(int color);
    int  getBarcodeColor()const;
    int  getSymbology()const; 
    void setBorderType(int borderTypeIndex/*enum BorderType*/);
    int  getBorderType()const;
    void setSymbology(int );
    std::string getBarcodeName()const;
    void setZoom(float);
    float getZoom()const;
    void setSHRT(bool hrt);
    bool getSHRT()const;
    void setRotateAngle(int angle);
    int  getRotateAngle()const;
    void onDraw(Canvas&canvas)override;
public:

};
}/*endof namespace*/
#endif/*ENABLE_BARCODE*/

