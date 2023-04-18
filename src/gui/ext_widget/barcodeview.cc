#if ENABLE_BARCODE
#include <zint.h>
#include <string.h>
#include <ext_widget/barcodeview.h>
#include <cdlog.h>

//REF:https://github.com/zint/zint-gpl-only/blob/master/backend_qt4/qzint.h/cpp

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
    mRotateAngle = 0;
    setBackgroundColor(Color::WHITE);
    mFgColor = Color::BLACK;
    mSymbol = ZBarcode_Create();
    mSymbol->outfile[0]=0;
    mSymbol->symbology = QRCode;
    mSymbol->whitespace_width =2;
    mSymbol->whitespace_height=2;
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
    if(mText!=text){
	mText = text;
        invalidate(true);
	encode();
    }
    LOGD("xdim=%f size=%.fx%.f %.fx%.f",ZBarcode_Default_Xdim(mSymbol->symbology),
		    mSymbol->width,mSymbol->height,
		    mSymbol->vector->width,mSymbol->vector->height);
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
    invalidate();
}

float  BarcodeView::getZoom()const{
    return mSymbol->scale;
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
	 height= heightSize;
    }else{
	 width = mSymbol->bitmap_width *mSymbol->scale;
	 height= mSymbol->bitmap_height*mSymbol->scale;;
    }
    setMeasuredDimension(width, height);
}

void BarcodeView::encode(){
    mErrorNo = ZBarcode_Encode_and_Buffer_Vector(mSymbol,(const unsigned char*)mText.c_str(),mText.length(),0);
    mErrorStr=std::string(mSymbol->errtxt);
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
    struct zint_vector_rect *rect;
    struct zint_vector_hexagon *hex;
    struct zint_vector_circle *circle;

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
        canvas.restore();
	LOGE("error [%d] %s",mErrorNo,mErrorStr.c_str());
        //return;
    }

    float xtr = paintRect.left;
    float ytr = paintRect.top;
    float scale;

    const float gwidth = mSymbol->vector->width;
    const float gheight = mSymbol->vector->height;

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
        canvas.rotate(mRotateAngle);
        canvas.translate(-paintRect.width / 2.0, -paintRect.height / 2.0); // Undo
    }

    canvas.translate(xtr, ytr);
    canvas.scale(scale, scale);

    const int m_fgColor=0xFF000000;

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
	    Color fgc(rect->colour == -1?m_fgColor:colourToCDColor(rect->colour));
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
        Color fgc(m_fgColor);
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
        Color fgc(m_fgColor);
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
                //p.setColor(m_fgColor);
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
        Color fgc(m_fgColor);
        canvas.set_source_rgb(fgc.red(),fgc.green(),fgc.blue());//p.setColor(m_fgColor);
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

