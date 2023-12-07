#if ENABLE_BARCODE
#include <zint.h>
#include <string.h>
#include <ext_widget/barcodeview.h>
#include <cdlog.h>

//REF:https://github.com/zint/zint-gpl-only/blob/master/backend_qt4/qzint.h/cpp

namespace cdroid{

DECLARE_WIDGET(BarcodeView)

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
    mErrorNo = 0;
    mRotateAngle = 0;
    mSymbology = QRCode;
    mDotty  = false;
    mShowHRT= true;
    mCmyk = true;
    mGssep= true;
    mQuietZones  = true;
    mNoQuietZones= true;
    mCompliantHeight= true;
    mReaderInit = true;
    mDebug = false;
    mGS1Parens = true;
    mGS1NoCheck= true;
    mBorderType= BOX;
    mOption1 = 0;
    mOption2 = 0;
    mOption3 = 0;
    mDotSize =1;
    mBorderWidth= 0;
    mWhiteSpace = 0;
    mVWhiteSpace= 0;
    mWarnLevel  = 0;
    mECI=0;
    mInputMode=DATA_MODE;
    mZoom = 1.f;
    mDpmm =.0f;
    mGuardDescent=.0f;
 
    setBackgroundColor(Color::WHITE);
    mFgColor = Color::BLACK;
    mSymbol  = ZBarcode_Create();
    mSymbol->outfile[0]= 0;
    mSymbol->show_hrt  = 1;
    mSymbol->border_width=2;
    mSymbol->whitespace_width = 2;
    mSymbol->whitespace_height= 2;
    mSymbol->scale = 2.f;
    mChanged = 0;
}

bool BarcodeView::resetSymbol(){
    mErrorNo = 0;
    mErrorStr.clear();

    if (mSymbol) {
        ZBarcode_Clear(mSymbol);
    } else if (!(mSymbol = ZBarcode_Create())) {
        mErrorNo = ZINT_ERROR_MEMORY;
        mErrorStr ="Insufficient memory for Zint structure";
        return false;
    }
    mSymbol->symbology = mSymbology;
    //mSymbol->height = m_height;
    mSymbol->scale = mZoom;
    mSymbol->whitespace_width = mWhiteSpace;
    mSymbol->whitespace_height= mVWhiteSpace;
    mSymbol->border_width = mBorderWidth;
    mSymbol->output_options = mBorderType;// | m_fontSetting;
    if (mDotty) {
        mSymbol->output_options |= BARCODE_DOTTY_MODE;
    }
    if (mCmyk) {
        mSymbol->output_options |= CMYK_COLOUR;
    }
    if (mGssep) {
        mSymbol->output_options |= GS1_GS_SEPARATOR;
    }
    if (mQuietZones) {
        mSymbol->output_options |= BARCODE_QUIET_ZONES;
    }
    if (mNoQuietZones) {
        mSymbol->output_options |= BARCODE_NO_QUIET_ZONES;
    }
    if (mCompliantHeight) {
        mSymbol->output_options |= COMPLIANT_HEIGHT;
    }
    if (mReaderInit) {
        mSymbol->output_options |= READER_INIT;
    }
    /*strcpy(mSymbol->fgcolour, mFgColor.name().toLatin1().right(6));
    if (mFgColor.alpha() != 0xFF) {
        strcat(mSymbol->fgcolour, mFgColor.name(QColor::HexArgb).toLatin1().mid(1,2));
    }
    strcpy(mSymbol->bgcolour, m_bgColor.name().toLatin1().right(6));
    if (m_bgColor.alpha() != 0xFF) {
        strcat(mSymbol->bgcolour, mFgColor.name(QColor::HexArgb).toLatin1().mid(1,2));
    }*/
    //strcpy(mSymbol->primary, mPrimaryMessage.toLatin1().left(127));
    mSymbol->option_1 = mOption1;
    mSymbol->option_2 = mOption2;
    mSymbol->option_3 = mOption3;
    mSymbol->show_hrt = mShowHRT ? 1 : 0;
    mSymbol->input_mode = mInputMode;
    if (mGS1Parens) {
        mSymbol->input_mode |= GS1PARENS_MODE;
    }
    if (mGS1NoCheck) {
        mSymbol->input_mode |= GS1NOCHECK_MODE;
    }
    mSymbol->eci = mECI;
    mSymbol->dpmm = mDpmm;
    mSymbol->dot_size = mDotSize;
    mSymbol->guard_descent = mGuardDescent;
    //mSymbol->structapp = m_structapp;
    mSymbol->warn_level = mWarnLevel;
    mSymbol->debug = mDebug ? ZINT_DEBUG_PRINT : 0;
    return true;
}


void BarcodeView::setBorderType(int borderTypeIndex){
    if(mBorderType!=borderTypeIndex){
        mBorderType = borderTypeIndex;
        mSymbol->output_options=borderTypeIndex;
	mChanged++;
    }
}

int  BarcodeView::getBorderType()const{
     return mBorderType;
}

int BarcodeView::getSymbology()const{
    
    return mSymbol->symbology;
}

void BarcodeView::setSymbology(int code){
    const int rc=ZBarcode_ValidID(code);
    LOGE_IF(!rc,"%d is not an valid Symbologies",code);
    if(rc!=0){
	mSymbology = code;
        mSymbol->symbology = code;
        if(!mText.empty()) invalidate();
    }
}

std::string BarcodeView::getBarcodeName()const{
    char name[32];
    ZBarcode_BarcodeName(mSymbol->symbology,name);
    return std::string(name);
}

void BarcodeView::setText(const std::string&text){
    if(mText!=text){
	mText = text;
	mSegs.clear();
	encode();
        invalidate(true);
    }
    float w,h;
    getWidthHeightXdim(2.f,w,h);
    LOGD("%p symbology=%d size=%.fx%.f /%.fx%.f/%.fx%.f",this,mSymbol->symbology,
		    mSymbol->width,mSymbol->height,w,h,
		    (mSymbol->vector?mSymbol->vector->width:0),
		    (mSymbol->vector?mSymbol->vector->height:0));
}

void BarcodeView::setBarcodeColor(int color){
    if(mFgColor!=color){
	mFgColor = color;
        invalidate();
    }
}

int BarcodeView::getBarcodeColor()const{
    return mFgColor;
}

void BarcodeView::setSHRT(bool hrt){
    mSymbol->show_hrt=(hrt?1:0);
    invalidate(true);
}

bool BarcodeView::getSHRT()const{
    return mSymbol->show_hrt;
}

void  BarcodeView::setZoom(float zoom){
    mSymbol->scale = zoom;
    mZoom = zoom;
    invalidate();
}

float  BarcodeView::getZoom()const{
    return mZoom;
}

void BarcodeView::setRotateAngle(int angle){
    angle%=360;
    if (angle == 90) {
        mRotateAngle = 90;
    } else if (angle == 180) {
        mRotateAngle = 180;
    } else if (angle == 270) {
        mRotateAngle = 270;
    } else {
        mRotateAngle = 0;
    }
}

int  BarcodeView::getRotateAngle()const{
    return mRotateAngle;
}

/* Input segments. */
std::vector<BarcodeView::ZintSeg> BarcodeView::getSegs() const{
    return mSegs;
}

/* Set segments. Note: clears text and sets eci */
void BarcodeView::setSegs(const std::vector<ZintSeg>& segs){
    mSegs= segs;
}

bool BarcodeView::hasHRT()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_HRT);
}

bool BarcodeView::isStackable()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_STACKABLE);
}

bool BarcodeView::isExtendable()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_EXTENDABLE);
}

bool BarcodeView::isComposite()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_COMPOSITE);
}

bool BarcodeView::supportsECI()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_ECI);
}

bool BarcodeView::supportsGS1()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_GS1);
}

bool BarcodeView::isDotty()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_DOTTY);
}

bool BarcodeView::hasDefaultQuietZones()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_QUIET_ZONES);
}

bool BarcodeView::isFixedRatio()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_FIXED_RATIO);
}

bool BarcodeView::supportsReaderInit()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_READER_INIT);
}

bool BarcodeView::supportsFullMultibyte()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_FULL_MULTIBYTE);
}

bool BarcodeView::hasMask()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_MASK);
}

bool BarcodeView::supportsStructApp()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_STRUCTAPP);
}

bool BarcodeView::hasCompliantHeight()const{
    return ZBarcode_Cap(mSymbol->symbology, ZINT_CAP_COMPLIANT_HEIGHT);
}

bool BarcodeView::getWidthHeightXdim(float x_dim, float &width_x_dim, float &height_x_dim) const{
    const float m_scale = mSymbol->scale;
    const float m_vectorWidth = mSymbol->vector->width;
    const float m_vectorHeight= mSymbol->vector->height;
    if (m_scale == 0.0f || m_vectorWidth == 0.0f || m_vectorHeight == 0.0f) {
        width_x_dim = height_x_dim = 0.0f;
        return false;
    }

    const float scale = m_scale * 2.0f;
    const float width = m_vectorWidth / scale;
    const float height = m_vectorHeight / scale;

    if ( (mRotateAngle == 90) || (mRotateAngle == 270) ) { // Sideways - swop
        width_x_dim = (height * x_dim);
        height_x_dim= (width * x_dim);
    } else {
        width_x_dim = (width * x_dim);
        height_x_dim= (height * x_dim);
    }
    return true;   
}

void  BarcodeView::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    const int widthMode  = MeasureSpec::getMode(widthMeasureSpec);
    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    const int widthSize  = MeasureSpec::getSize(widthMeasureSpec);
    const int heightSize = MeasureSpec::getSize(heightMeasureSpec);
    int width,height;
    if(widthMode == MeasureSpec::EXACTLY){
	 width = widthSize;
    }else{	    
	 if(mSymbol->vector){
	     width = mSymbol->vector->width * mSymbol->scale;
	 }else{
	     width = mSymbol->width * mSymbol->scale;
	 }
    }
    if(heightMode == MeasureSpec::EXACTLY){
	 height= heightSize;
    }else{
	 if(mSymbol->vector){
	     height= mSymbol->vector->height* mSymbol->scale;
	 }else{
	     height= mSymbol->height* mSymbol->scale;
	 }
    }
    LOGD("setMeasuredDimension(%d,%d)",width,height);
    setMeasuredDimension(width, height);
}

#define MAX_SEGS 256
int BarcodeView::convertSegs(struct zint_seg zsegs[], std::vector<std::string>& bstrs) {
    int i = 0;
    for (i = 0; i < (int) mSegs.size() && i < MAX_SEGS && !mSegs[i].mText.empty(); i++) {
        zsegs[i].eci = mSegs[i].mECI;
        bstrs.push_back(mSegs[i].mText);
        zsegs[i].source = (unsigned char *)mSegs[i].mText.c_str();
        zsegs[i].length = mSegs[i].mText.length();
    }
    return i;
}

void BarcodeView::encode(){
    if(resetSymbol()){
        if (mSegs.empty()) {
            /* Note do our own rotation */
            mErrorNo = ZBarcode_Encode_and_Buffer_Vector(mSymbol, (unsigned char*) mText.c_str(), mText.length(), 0);
        } else {
            struct zint_seg segs[MAX_SEGS];
            std::vector<std::string> bstrs;
            const int seg_count = convertSegs(segs, bstrs);
            /* Note do our own rotation */
            mErrorNo = ZBarcode_Encode_Segs_and_Buffer_Vector(mSymbol, segs, seg_count, 0);
        }
        mErrorStr = mSymbol->errtxt;
    }
    if (mErrorNo < ZINT_ERROR) {
        mBorderType = mSymbol->output_options & (BARCODE_BIND | BARCODE_BOX | BARCODE_BIND_TOP);
        //m_height = mSymbol->height;
        mBorderWidth = mSymbol->border_width;
        mWhiteSpace  = mSymbol->whitespace_width;
        mVWhiteSpace = mSymbol->whitespace_height;
        mEncodedWidth = mSymbol->width;
        mEncodedRows  = mSymbol->rows;
        mEncodedHeight= mSymbol->height;
        mVectorWidth  = mSymbol->vector->width;
        mVectorHeight = mSymbol->vector->height;
        //emit encoded();
    } else {
        mEncodedWidth = mEncodedRows = 0;
        mEncodedHeight= mVectorWidth = mVectorHeight = 0.0f;
        //emit errored();
    }
}

/* Convert `zint_vector_rect->colour` to Qt color */
static int colourToCDColor(int colour) {
    switch (colour) {
    case 1: return Color::CYAN;   break;
    case 2: return Color::BLUE;   break;
    case 3: return Color::MAGENTA;break;
    case 4: return Color::RED   ; break;
    case 5: return Color::YELLOW; break;
    case 6: return Color::GREEN ; break;
    case 8 :return Color::WHITE ; break;
    default:return Color::BLACK ; break;
    }
}

/* Helper to calculate max right and bottom of elements for fudging render() */
static void getMaxRectsRightBottom(struct zint_vector *vector, int &maxRight, int &maxBottom) {
    const struct zint_vector_rect *rect;
    const struct zint_vector_hexagon *hex;
    const struct zint_vector_circle *circle;

    maxRight = maxBottom = -1;

    for (rect = vector->rectangles; rect; rect = rect->next) {
        if (rect->x + rect->width > maxRight) {
            maxRight = rect->x + rect->width;
        }
        if (rect->y + rect->height > maxBottom) {
            maxBottom = rect->y + rect->height;
        }
    }

    for (hex = vector->hexagons; hex; hex = hex->next) {
        if (hex->x + hex->diameter > maxRight) {
            maxRight = hex->x + hex->diameter;
        }
        if (hex->y + hex->diameter > maxBottom) {
            maxBottom = hex->y + hex->diameter;
        }
    }

    for (circle = vector->circles; circle; circle = circle->next) {
        if (circle->x + circle->diameter + circle->width > maxRight) {
            maxRight = circle->x + circle->diameter + circle->width;
        }
        if (circle->y + circle->diameter + circle->width > maxBottom) {
            maxBottom = circle->y + circle->diameter + circle->width;
        }
    }
    // TODO: Strings?
}

void  BarcodeView::onDraw(Canvas&canvas){
    View::onDraw(canvas);
    const struct zint_vector_rect *rect;
    const struct zint_vector_hexagon *hex;
    const struct zint_vector_circle *circle;
    struct zint_vector_string *string;
    const RectF paintRect ={0,0,(float)getWidth(),(float)getHeight()};

    canvas.save();
    if (mErrorNo >= ZINT_ERROR) {
        canvas.set_font_size(14);
        canvas.show_text(mErrorStr);//drawText(paintRect, Qt::AlignCenter | Qt::TextWordWrap, m_lastError);
	canvas.fill();
        canvas.restore();
        return;
    }

    float xtr = paintRect.left;
    float ytr = paintRect.top;
    float scale = mSymbol->scale;

    const float gwidth = mSymbol->vector->width;
    const float gheight= mSymbol->vector->height;
#if 0
    if (mRotateAngle == 90 || mRotateAngle == 270) {
        if (paintRect.width / gheight < paintRect.height / gwidth) {
            scale = paintRect.width / gheight;
        } else {
            scale = paintRect.height / gwidth;
        }
    } else {
        if (paintRect.width / gwidth < paintRect.height / gheight) {
            scale = paintRect.width / gwidth;
        } else {
            scale = paintRect.height / gheight;
        }
    }

    xtr += (float) (paintRect.width - gwidth * scale) / 2.0f;
    ytr += (float) (paintRect.height - gheight * scale) / 2.0f;

    if (mRotateAngle) {
        canvas.translate(paintRect.width / 2.0, paintRect.height / 2.0); // Need to rotate around centre
        canvas.rotate_degreenis(mRotateAngle);
        canvas.translate(-paintRect.width / 2.0, -paintRect.height / 2.0); // Undo
    }

#endif
    canvas.translate(xtr, ytr);
    canvas.scale(scale, scale);
    //QBrush bgBrush(m_bgColor);
    //canvas.rectangle(0, 0, gwidth, gheight);//, bgBrush);
    //background ic drawed by View

    // Plot rectangles
    rect = mSymbol->vector->rectangles;
    if (rect) {
        int maxRight = -1, maxBottom = -1; // Used for fudging below
        getMaxRectsRightBottom(mSymbol->vector, maxRight, maxBottom);
        canvas.set_antialias(Cairo::ANTIALIAS_NONE);
        while (rect) {
	    Color fgc(rect->colour == -1?mFgColor:colourToCDColor(rect->colour));
	    LOGV("rect %p color=%x",rect,rect->colour);
	    canvas.set_source_rgb(fgc.red(),fgc.green(),fgc.blue());
            // Allow for rounding errors on translation/scaling TODO: proper calc
            float fudgeW = rect->x + rect->width == maxRight ? 0.1f : 0.0f;
            float fudgeH = rect->y + rect->height == maxBottom ? 0.1f : 0.0f;
            canvas.rectangle(rect->x, rect->y, rect->width + fudgeW, rect->height + fudgeH);
	    canvas.fill();
            rect = rect->next;
        }
    }


    // Plot hexagons
    hex = mSymbol->vector->hexagons;
    canvas.set_antialias(Cairo::ANTIALIAS_DEFAULT);
    if (hex) {
        Color fgc(mFgColor);
        float previous_diameter = 0.0, radius = 0.0, half_radius = 0.0, half_sqrt3_radius = 0.0;
	canvas.set_source_rgb(fgc.red(),fgc.green(),fgc.blue());
        while (hex) {
            if (previous_diameter != hex->diameter) {
                previous_diameter = hex->diameter;
                radius = 0.5 * previous_diameter;
                half_radius = 0.25 * previous_diameter;
                half_sqrt3_radius = 0.43301270189221932338 * previous_diameter;
            }

            canvas.move_to(hex->x, hex->y + radius);
            canvas.line_to(hex->x + half_sqrt3_radius, hex->y + half_radius);
            canvas.line_to(hex->x + half_sqrt3_radius, hex->y - half_radius);
            canvas.line_to(hex->x, hex->y - radius);
            canvas.line_to(hex->x - half_sqrt3_radius, hex->y - half_radius);
            canvas.line_to(hex->x - half_sqrt3_radius, hex->y + half_radius);
            canvas.line_to(hex->x, hex->y + radius);
            canvas.fill();

            hex = hex->next;
        }
    }

    // Plot dots (circles)
    circle = mSymbol->vector->circles;
    if (circle) {
        Color fgc(mFgColor);
        float previous_diameter = 0.0, radius = 0.0;
	canvas.set_source_rgb(fgc.red(),fgc.green(),fgc.blue());
        while (circle) {
            if (previous_diameter != circle->diameter) {
                previous_diameter = circle->diameter;
                radius = 0.5 * previous_diameter;
            }
	    canvas.arc(circle->x, circle->y,radius,0,M_PI*2.0);
	    canvas.set_line_width(circle->width);
	    LOGD("circle %p color=%x",circle,circle->colour);
            if (circle->colour) { // Set means use background colour
                //canvas.set_source_rgb();//p.setColor(m_bgColor);
                //p.setWidthF(circle->width);
		if(circle->width==0)canvas.fill_preserve();
		canvas.stroke();
                //canvas.setPen(p);
                //canvas.setBrush(circle->width ? Qt::NoBrush : bgBrush);
            } else {
                //p.setColor(mFgColor);
                //p.setWidthF(circle->width);
                //canvas.setPen(p);
                //canvas.setBrush(circle->width ? Qt::NoBrush : fgBrush);
            }
            //canvas.drawEllipse(QPointF(circle->x, circle->y), radius, radius);
            circle = circle->next;
        }
    }

    // Plot text
    string = mSymbol->vector->strings;
    if (string) {
        Color fgc(mFgColor);
        canvas.set_source_rgb(fgc.red(),fgc.green(),fgc.blue());//p.setColor(mFgColor);
        bool bold = (mSymbol->output_options & BOLD_TEXT) && (!isExtendable() || (mSymbol->output_options & SMALL_TEXT));
        while (string) {
            canvas.set_font_size(string->fsize);
            //canvas.setFont(font);
	    const std::string content((const char *) string->text);
            /* string->y is baseline of font */
            if (string->halign == 1) { /* Left align */
		canvas.move_to(string->x, string->y);
            } else {
		Cairo::TextExtents extents;
		canvas.get_text_extents(content,extents);
                const int text_width = extents.width;
                if (string->halign == 2) { /* Right align */
		    canvas.move_to(string->x - text_width, string->y);
                } else { /* Centre align */
	            canvas.move_to(string->x - (text_width / 2.0), string->y);
                }
            }
	    canvas.show_text(content);
            string = string->next;
        }
    }

    canvas.restore();
}

}/*endof namespace*/

#endif/*ENABLE_BARCODE*/

