#include <drawables/ninepatch.h>
#include <core/context.h>
#include <drawables/bitmapdrawable.h>
#include <cdlog.h>

using namespace Cairo;

/*https://github.com/Roninsc2/NinePatchQt/blob/master/ninepatch.cpp*/
namespace cdroid{

NinePatch::NinePatch(Cairo::RefPtr<ImageSurface> image)
    : mImage(image){
    mContentArea = getContentArea();
    mOpacity = INT_MAX;
    mAlpha =1.f;
    getResizeArea();
    if (!mResizeDistancesX.size() || !mResizeDistancesY.size()) {
        //throw new ExceptionNot9Patch;
        throw "Not ninepatch image!";
    }
}

NinePatch::NinePatch(Context*ctx,const std::string&resid){
    mImage= ctx->loadImage(resid);
    mContentArea = getContentArea();
    mOpacity = INT_MAX;
    mAlpha = 1.0f;
    getResizeArea();
    if (!mResizeDistancesX.size() || !mResizeDistancesY.size()) {
        //throw new ExceptionNot9Patch;
        throw "Not ninepatch image!";
    }
}

NinePatch::~NinePatch() {
}

static int getRotateAngle(Canvas&canvas){
    Cairo::Matrix ctx=canvas.get_matrix();
    double radians = atan2(ctx.yy, ctx.xy);
    return int(radians*180.f/M_PI);
}

void NinePatch::draw(Canvas& painter, int  x, int  y,float alpha) {
    const int angle_degrees = getRotateAngle(painter);
    painter.save();
    painter.translate(x,y);
    if(mOpacity ==INT_MAX){
        mOpacity = BitmapDrawable::computeTransparency(mImage);
    }
    mAlpha = alpha;
    const Cairo::SurfacePattern::Filter filterMode = (angle_degrees%90==0)&&(mOpacity==PixelFormat::OPAQUE)?SurfacePattern::Filter::NEAREST:SurfacePattern::Filter::BILINEAR;
    painter.set_source(mCachedImage,0,0);
    painter.rectangle(0,0,mCachedImage->get_width(),mCachedImage->get_height());
    painter.clip();
    painter.paint();
    painter.restore();
}

void NinePatch::draw(Canvas& painter, const Rect&rect,float alpha){
    const int angle_degrees = getRotateAngle(painter);
    int resizeWidth = 0,resizeHeight = 0;
    const int width = rect.width;
    const int height= rect.height;
    std::ostringstream oss;
    painter.save();
    painter.translate(rect.left,rect.top);
    if(mOpacity ==INT_MAX){
        mOpacity = BitmapDrawable::computeTransparency(mImage);
    }
    mAlpha = alpha;
    const Cairo::SurfacePattern::Filter filterMode = (angle_degrees%90==0)&&(mOpacity==PixelFormat::OPAQUE)?SurfacePattern::Filter::NEAREST:SurfacePattern::Filter::BILINEAR;
    for (int i = 0; i < mResizeDistancesX.size(); i++) {
        resizeWidth += mResizeDistancesX[i].second;
    }
    for (int i = 0; i < mResizeDistancesY.size(); i++) {
        resizeHeight += mResizeDistancesY[i].second;
    }

    if (width < (mImage->get_width() - 2 - resizeWidth) && height < (mImage->get_height() - 2 - resizeHeight)) {
        oss<<"IncorrectWidth("<<width<<") must>="<<mImage->get_width()<<"(image.width)-2-"<<resizeWidth<<"(resizeWidth) && incorrectHeight("
                <<height<<")>="<<mImage->get_height()<<"(image.height)-2-"<<resizeHeight<<"(resizeHeight))";
    }
    if (width < (mImage->get_width() - 2 - resizeWidth)) {
        oss<<"IncorrectWidth("<<width<<"must>="<<mImage->get_width()<<"image.width)-2-"<<resizeWidth<<"(resizeWidth)";
    }
    if (height < (mImage->get_height() - 2 - resizeHeight)) {
        oss<<"IncorrectHeight("<<height<<"must>="<<mImage->get_height()<<"(image.height)-2-"<<resizeHeight<<"(resizeHeight)";
    }
    const bool hasErrors = (oss.str().empty()==false);
    LOGE_IF(hasErrors,"%s",oss.str());
    mWidth = rect.width;
    mHeight= rect.height;
	painter.save();
	painter.translate(rect.left,rect.top);
    updateCachedImage(mWidth,mHeight,&painter);
	painter.restore();
}

void NinePatch::setImageSize(int width, int height) {
    int resizeWidth = 0;
    int resizeHeight = 0;
    std::ostringstream oss;
    if((mWidth == width) && (mHeight==height))return;
    for (int i = 0; i < mResizeDistancesX.size(); i++) {
        resizeWidth += mResizeDistancesX[i].second;
    }
    for (int i = 0; i < mResizeDistancesY.size(); i++) {
        resizeHeight += mResizeDistancesY[i].second;
    }
    if (width < (mImage->get_width() - 2 - resizeWidth) && height < (mImage->get_height() - 2 - resizeHeight)) {
        oss<<"IncorrectWidth("<<width<<") must>="<<mImage->get_width()<<"(image.width)-2-"<<resizeWidth<<"(resizeWidth) && incorrectHeight("
		<<height<<")>="<<mImage->get_height()<<"(image.height)-2-"<<resizeHeight<<"(resizeHeight))";
    }
    if (width < (mImage->get_width() - 2 - resizeWidth)) {
		oss<<"IncorrectWidth("<<width<<"must>="<<mImage->get_width()<<"image.width)-2-"<<resizeWidth<<"(resizeWidth)";
    }
    if (height < (mImage->get_height() - 2 - resizeHeight)) {
        oss<<"IncorrectHeight("<<height<<"must>="<<mImage->get_height()<<"(image.height)-2-"<<resizeHeight<<"(resizeHeight)";
    }
    if(oss.str().empty()==false)
        LOG(ERROR)<<oss.str();
    if (width != mWidth || height != mHeight) {
        mWidth = width;
        mHeight = height;
        updateCachedImage(width, height,nullptr);
    }
}

Rect NinePatch::getContentArea(int  width, int  height) {
    return Rect{mContentArea.left, mContentArea.top, (width - (mImage->get_width() - 2 -mContentArea.width)),
                  (height - (mImage->get_height() - 2 -mContentArea.height))};
}

Rect NinePatch::getPadding()const{
    return mPadding;
}

void NinePatch::drawScaledPart(const Rect& oldRect, const Rect& newRect,Cairo::Context&painter) {
    if (newRect.width && newRect.height) {
        const double scaleX=(double)newRect.width/oldRect.width;
        const double scaleY=(double)newRect.height/oldRect.height;
        double dx = newRect.left;
        double dy = newRect.top;
        painter.save();
        painter.rectangle(dx,dy,newRect.width,newRect.height);
        if( (newRect.width!=oldRect.width) || (newRect.height!=oldRect.height) ){
            painter.scale(scaleX,scaleY);
	        dx/=scaleX;
            dy/=scaleY;
        }
        painter.clip();
        painter.set_source(mImage,dx-oldRect.left,dy-oldRect.top);
        /*default filtertype:Good cannot be use here*/
        Cairo::RefPtr<SurfacePattern>spat = painter.get_source_for_surface();
        if(spat)spat->set_filter(SurfacePattern::Filter::NEAREST);
        painter.paint_with_alpha(mAlpha);
        painter.restore();
    }
}

void NinePatch::drawConstPart(const Rect& oldRect, const Rect& newRect,Cairo::Context&painter) {
    painter.save();
    painter.rectangle(newRect.left,newRect.top,newRect.width,newRect.height);
    painter.clip();
    painter.set_source(mImage,newRect.left-oldRect.left,newRect.top-oldRect.top);
    Cairo::RefPtr<SurfacePattern>spat = painter.get_source_for_surface();
    if(spat)spat->set_filter(SurfacePattern::Filter::FAST);
    painter.paint_with_alpha(mAlpha);
    painter.restore();
}

static inline bool IsColorBlack(Cairo::RefPtr<ImageSurface>img,int i,int j) {
    unsigned char*data=img->get_data()+img->get_stride()*j+i*4;
    uint8_t r = data[0];
    uint8_t g = data[1];
    uint8_t b = data[2];
    uint8_t a = data[3];
    if (a < 128) return false;
    return (r < 128 && g < 128 && b < 128);
}

Rect NinePatch::getContentArea() {
    int  j = mImage->get_height() - 1;
    int  left = 0 ,  right = 0;

    for(int  i = 0; i < mImage->get_width() ; i++) {
        if (IsColorBlack(mImage,i, j) && left == 0) {
            left = i;
        } else {
            if (left != 0 && IsColorBlack(mImage,i, j)) {
                right = i;
            }
        }
    }
    if (left && !right)right = left;
    left -= 1;

    int  i = mImage->get_width() - 1;
    int  top =0 ,  bot = 0;
    for(int  j = 0; j < mImage->get_height() ; j++) {
        if (IsColorBlack(mImage,i, j)&& top == 0) {
            top = j;
        } else {
            if (top && IsColorBlack(mImage,i, j)) {
                bot = j;
            }
        }
    }
    if (top && !bot) bot = top;
    top -= 1;

    mPadding.set(left, top , mImage->get_width()-right-2, mImage->get_height()-2-bot);
    LOGV("%p padding=(%d,%d,%d,%d)",this,left, top,mPadding.width,mPadding.height);
    return Rect{left, top, right - left, bot - top};
}

void NinePatch::getResizeArea() {
    int  j = 0;
    int  left = 0, right = 0;
    for(int  i = 0; i < mImage->get_width(); i++) {
        if (IsColorBlack(mImage,i, j) && left == 0) {
            left = i;
        }
        if (left && IsColorBlack(mImage,i, j) && !IsColorBlack(mImage,i+1, j)) {
            right = i;
            left -= 1;
            mResizeDistancesX.push_back(std::make_pair(left, right - left));
            right = 0;
            left = 0;
        }
    }
    int  i = 0;
    int  top = 0, bot = 0;
    for(int  j = 0; j < mImage->get_height(); j++) {
        if (IsColorBlack(mImage,i, j) && top == 0) {
            top = j;
        }
        if (top && IsColorBlack(mImage,i, j) && !IsColorBlack(mImage,i, j+1)) {
            bot = j;
            top -= 1;
            mResizeDistancesY.push_back(std::make_pair(top, bot - top));
            top = 0;
            bot = 0;
        }
    }
}

void NinePatch::getFactor(int width, int height, double& factorX, double& factorY) {
    int topResize = width - (mImage->get_width() - 2);
    int leftResize = height - (mImage->get_height() - 2);
    for (int i = 0; i < mResizeDistancesX.size(); i++) {
        topResize += mResizeDistancesX[i].second;
        factorX += mResizeDistancesX[i].second;
    }
    for (int i = 0; i < mResizeDistancesY.size(); i++) {
        leftResize += mResizeDistancesY[i].second;
        factorY += mResizeDistancesY[i].second;
    }
    factorX = (double)topResize / factorX;
    factorY = (double)leftResize / factorY;
}

void NinePatch::updateCachedImage(int width, int height,Cairo::Context*painterIn) {
    double lostX  = 0.f, lostY  = 0.f;
    double factorX= 0.f, factorY= 0.f;
    int x1 = 0 , y1 = 0; //for image parts X/Y
    int widthResize,heightResize; //width/height for image parts
    int resizeX = 0 , resizeY = 0;
    int offsetX = 0 , offsetY = 0;
    
    RefPtr<Cairo::Context> imgPainter;
    Cairo::Context*ppainter = painterIn;
    if(painterIn==nullptr){
        mCachedImage = ImageSurface::create(Surface::Format::ARGB32,width,height);
        imgPainter=Cairo::Context::create(mCachedImage);
		imgPainter->save();
        imgPainter->set_operator(Cairo::Context::Operator::CLEAR);
        imgPainter->rectangle(0,0,width,height);
        imgPainter->fill();
		imgPainter->restore();
        ppainter=imgPainter.get();
    }
    Cairo::Context&painter=*imgPainter.get();
    getFactor(width, height, factorX, factorY);
    for (int  i = 0; i < mResizeDistancesX.size(); i++) {
        y1 = 0;
        offsetY = 0;
        lostY = 0.0;
        for (int  j = 0; j < mResizeDistancesY.size(); j++) {
            widthResize = mResizeDistancesX[i].first - x1;
            heightResize = mResizeDistancesY[j].first - y1;

            drawConstPart(Rect{x1 + 1, y1 + 1, widthResize, heightResize},
                 Rect{x1 + offsetX, y1 + offsetY, widthResize, heightResize}, painter);

            int  y2 = mResizeDistancesY[j].first;
            heightResize = mResizeDistancesY[j].second;
            resizeY = round((double)heightResize * factorY);
            lostY += resizeY - ((double)heightResize * factorY);
            if (fabs(lostY) >= 1.f) {
                if (lostY < 0) {  resizeY += 1;   lostY += 1.0; }
                else { resizeY -= 1;  lostY -= 1.0; }
            }
            drawScaledPart(Rect{x1 + 1, y2 + 1, widthResize, heightResize},
                Rect{x1 + offsetX, y2 + offsetY, widthResize, resizeY}, painter);

            int  x2 = mResizeDistancesX[i].first;
            widthResize = mResizeDistancesX[i].second;
            heightResize = mResizeDistancesY[j].first - y1;
            resizeX = round((double)widthResize * factorX);
            lostX += resizeX - ((double)widthResize * factorX);
            if (fabs(lostX) >= 1.f) {
                if (lostX < 0) { resizeX += 1; lostX += 1.0;}
                else { resizeX -= 1; lostX -= 1.0; }
            }
            drawScaledPart(Rect{x2 + 1, y1 + 1, widthResize, heightResize},
                Rect{x2 + offsetX, y1 + offsetY, resizeX, heightResize}, painter);

            heightResize = mResizeDistancesY[j].second;
            drawScaledPart(Rect{x2 + 1, y2 + 1, widthResize, heightResize},
                Rect{x2 + offsetX, y2 + offsetY, resizeX, resizeY}, painter);

            y1 = mResizeDistancesY[j].first + mResizeDistancesY[j].second;
            offsetY += resizeY - mResizeDistancesY[j].second;
        }
        x1 = mResizeDistancesX[i].first + mResizeDistancesX[i].second;
        offsetX += resizeX - mResizeDistancesX[i].second;
    }
    x1 = mResizeDistancesX[mResizeDistancesX.size() - 1].first + mResizeDistancesX[mResizeDistancesX.size() - 1].second;
    widthResize = mImage->get_width() - x1 - 2;
    y1 = 0;
    lostX = 0.0;
    lostY = 0.0;
    offsetY = 0;
    for (int i = 0; i < mResizeDistancesY.size(); i++) {
        drawConstPart(Rect{x1 + 1, y1 + 1, widthResize, mResizeDistancesY[i].first - y1},
            Rect{x1 + offsetX, y1 + offsetY, widthResize, mResizeDistancesY[i].first - y1}, painter);
        y1 = mResizeDistancesY[i].first;
        resizeY = round((double)mResizeDistancesY[i].second * factorY);
        lostY += resizeY - ((double)mResizeDistancesY[i].second * factorY);
        if (fabs(lostY) >= 1.f) {
            if (lostY < 0) { resizeY += 1;  lostY += 1.0; }
            else { resizeY -= 1;  lostY -= 1.0; }
        }
        drawScaledPart(Rect{x1 + 1, y1 + 1, widthResize, mResizeDistancesY[i].second},
            Rect{x1 + offsetX, y1 + offsetY, widthResize, resizeY}, painter);
        y1 = mResizeDistancesY[i].first + mResizeDistancesY[i].second;
        offsetY += resizeY - mResizeDistancesY[i].second;
    }
    y1 = mResizeDistancesY[mResizeDistancesY.size() - 1].first + mResizeDistancesY[mResizeDistancesY.size() - 1].second;
    heightResize = mImage->get_height() - y1 - 2;
    x1 = 0;
    offsetX = 0;
    for (int i = 0; i < mResizeDistancesX.size(); i++) {
        drawConstPart(Rect{x1 + 1, y1 + 1, mResizeDistancesX[i].first - x1, heightResize},
            Rect{x1 + offsetX, y1 + offsetY, mResizeDistancesX[i].first - x1, heightResize}, painter);
        x1 = mResizeDistancesX[i].first;
        resizeX = round((double)mResizeDistancesX[i].second * factorX);
        lostX += resizeX - ((double)mResizeDistancesX[i].second * factorX);
        if (fabs(lostX) >= 1.f) {
            if (lostX < 0) {  resizeX += 1;  lostX += 1.0; }
            else { resizeX -= 1;  lostX += 1.0; }
        }
        drawScaledPart(Rect{x1 + 1, y1 + 1, mResizeDistancesX[i].second, heightResize},
            Rect{x1 + offsetX, y1 + offsetY, resizeX, heightResize}, painter);
        x1 = mResizeDistancesX[i].first + mResizeDistancesX[i].second;
        offsetX += resizeX - mResizeDistancesX[i].second;
    }
    x1 = mResizeDistancesX[mResizeDistancesX.size() - 1].first + mResizeDistancesX[mResizeDistancesX.size() - 1].second;
    widthResize = mImage->get_width() - x1 - 2;
    y1 = mResizeDistancesY[mResizeDistancesY.size() - 1].first + mResizeDistancesY[mResizeDistancesY.size() - 1].second;
    heightResize = mImage->get_height() - y1 - 2;
    drawConstPart(Rect{x1 + 1, y1 + 1, widthResize, heightResize},
         Rect{x1 + offsetX, y1 + offsetY, widthResize, heightResize}, painter);
}

}/*endof namespace*/
