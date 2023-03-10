#include <drawables/ninepatch.h>
using namespace Cairo;
namespace cdroid{

NinePatch::NinePatch(Cairo::RefPtr<ImageSurface> image)
    : mImage(image){
    mContentArea = getContentArea(false);
    getResizeArea();
    mPadding = getContentArea(true);
    if (!mResizeDistancesX.size() || !mResizeDistancesY.size()) {
        //throw new ExceptionNot9Patch;
	throw "Not ninepatch image!";
    }
}

NinePatch::~NinePatch() {
}

void NinePatch::draw(Canvas& painter, int  x, int  y) {
    //painter.drawImage(x, y, mCachedImage);
    painter.save();
    painter.set_source(mCachedImage,0,0);
    painter.rectangle(x,y,mCachedImage->get_width(),mCachedImage->get_height());
    painter.fill();
    painter.restore();
}

void NinePatch::setImageSize(int width, int height) {
    int resizeWidth = 0;
    int resizeHeight = 0;
    if((mWidth == width) && (mHeight==height))return;
    for (int i = 0; i < mResizeDistancesX.size(); i++) {
        resizeWidth += mResizeDistancesX[i].second;
    }
    for (int i = 0; i < mResizeDistancesY.size(); i++) {
        resizeHeight += mResizeDistancesY[i].second;
    }
    if (width < (mImage->get_width() - 2 - resizeWidth) && height < (mImage->get_height() - 2 - resizeHeight)) {
        //throw new ExceptionIncorrectWidthAndHeight(mImage->get_width() - 2 , mImage->get_height() - 2 );
    }
    if (width < (mImage->get_width() - 2 - resizeWidth)) {
        //throw new ExceptionIncorrectWidth(mImage->get_width() - 2 , mImage->get_height() - 2 );
    }
     if (height < (mImage->get_height() - 2 - resizeHeight)) {
        //throw new ExceptionIncorrectHeight(mImage->get_width() - 2 , mImage->get_height() - 2 );
    }
    if (width != mWidth || height != mHeight) {
        mWidth = width;
        mHeight = height;
        updateCachedImage(width, height);
    }
}

RECT NinePatch::getContentArea(int  width, int  height) {
    return RECT{mContentArea.left, mContentArea.top, (width - (mImage->get_width() - 2 -mContentArea.width)),
                  (height - (mImage->get_height() - 2 -mContentArea.height))};
}

RECT NinePatch::getPadding()const{
    return mPadding;
}

void NinePatch::drawScaledPart(const RECT& oldRect, const RECT& newRect,RefPtr<Cairo::Context> painter) {
    if (newRect.width && newRect.height) {
        /*QImage img = mImage->copy(oldRect);
        img = img.scaled(newRect.width, newRect.height);
        painter.drawImage(newRect.x(), newRect.y(), img, 0, 0, newRect.width, newRect.height);
	*/
	painter->save();
	painter->set_source(mImage,oldRect.left,oldRect.top);
	painter->scale((double)newRect.width/oldRect.width,(double)newRect.height/oldRect.height);
	painter->rectangle(newRect.left,newRect.top,newRect.width,newRect.height);
	painter->fill();
	painter->restore();
    }
}

void NinePatch::drawConstPart(const RECT& oldRect, const RECT& newRect,RefPtr<Cairo::Context> painter) {
    //QImage img = mImage->copy(oldRect);
    //painter.drawImage(newRect.x(), newRect.y(), img, 0, 0, newRect.width(), newRect.height());
    painter->save();
    painter->set_source(mImage,oldRect.left,oldRect.top);
    painter->rectangle(newRect.left,newRect.top,newRect.width,newRect.height);
    painter->fill();
    painter->restore();
}

inline bool IsColorBlack(Cairo::RefPtr<ImageSurface>img,int i,int j) {
    unsigned char*data=img->get_data()+img->get_stride()*j+i*4;
    uint8_t a = data[3];
    uint8_t r = data[0];
    uint8_t g = data[1];
    uint8_t b = data[2];
    //if (a < 128) return false;
    //return (r < 128 && g < 128 && b < 128);
    return a==255;
}

RECT NinePatch::getContentArea(bool padding) {
    int  j = padding?0:(mImage->get_height() - 1);
    int  left = 0;
    int  right = 0;
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
    int  i = padding?0:(mImage->get_width() - 1);
    int  top = 0;
    int  bot = 0;
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
    return RECT{left, top, right - left, bot - top};
}

void NinePatch::getResizeArea() {
    int  j = 0;
    int  left = 0;
    int  right = 0;
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
    int  top = 0;
    int  bot = 0;
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

void NinePatch::updateCachedImage(int width, int height) {
    double lostX  = 0.f, lostY  = 0.f;
    double factorX= 0.f, factorY= 0.f;
    int x1 = 0 , y1 = 0; //for image parts X/Y
    int widthResize,heightResize; //width/height for image parts
    int resizeX = 0 , resizeY = 0;
    int offsetX = 0 , offsetY = 0;

    mCachedImage =  ImageSurface::create(Surface::Format::ARGB32,width,height);
    RefPtr<Cairo::Context> painter=Cairo::Context::create(mCachedImage);
    painter->set_source_rgb(0,0,0);
    painter->rectangle(0,0,width,height);
    painter->fill();
    getFactor(width, height, factorX, factorY);
    for (int  i = 0; i < mResizeDistancesX.size(); i++) {
        y1 = 0;
        offsetY = 0;
        lostY = 0.0;
        for (int  j = 0; j < mResizeDistancesY.size(); j++) {
            widthResize = mResizeDistancesX[i].first - x1;
            heightResize = mResizeDistancesY[j].first - y1;

            drawConstPart(RECT{x1 + 1, y1 + 1, widthResize, heightResize},
                 RECT{x1 + offsetX, y1 + offsetY, widthResize, heightResize}, painter);

            int  y2 = mResizeDistancesY[j].first;
            heightResize = mResizeDistancesY[j].second;
            resizeY = round((double)heightResize * factorY);
            lostY += resizeY - ((double)heightResize * factorY);
            if (fabs(lostY) >= 1.f) {
                if (lostY < 0) {  resizeY += 1;   lostY += 1.0; }
	       	else { resizeY -= 1;  lostY -= 1.0; }
            }
            drawScaledPart(RECT{x1 + 1, y2 + 1, widthResize, heightResize},
                RECT{x1 + offsetX, y2 + offsetY, widthResize, resizeY}, painter);

            int  x2 = mResizeDistancesX[i].first;
            widthResize = mResizeDistancesX[i].second;
            heightResize = mResizeDistancesY[j].first - y1;
            resizeX = round((double)widthResize * factorX);
            lostX += resizeX - ((double)widthResize * factorX);
            if (fabs(lostX) >= 1.f) {
                if (lostX < 0) { resizeX += 1; lostX += 1.0;}
                else { resizeX -= 1; lostX -= 1.0; }
            }
            drawScaledPart(RECT{x2 + 1, y1 + 1, widthResize, heightResize},
                RECT{x2 + offsetX, y1 + offsetY, resizeX, heightResize}, painter);

            heightResize = mResizeDistancesY[j].second;
            drawScaledPart(RECT{x2 + 1, y2 + 1, widthResize, heightResize},
                RECT{x2 + offsetX, y2 + offsetY, resizeX, resizeY}, painter);

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
        drawConstPart(RECT{x1 + 1, y1 + 1, widthResize, mResizeDistancesY[i].first - y1},
            RECT{x1 + offsetX, y1 + offsetY, widthResize, mResizeDistancesY[i].first - y1}, painter);
        y1 = mResizeDistancesY[i].first;
        resizeY = round((double)mResizeDistancesY[i].second * factorY);
        lostY += resizeY - ((double)mResizeDistancesY[i].second * factorY);
        if (fabs(lostY) >= 1.f) {
            if (lostY < 0) { resizeY += 1;  lostY += 1.0; }
            else { resizeY -= 1;  lostY -= 1.0; }
        }
        drawScaledPart(RECT{x1 + 1, y1 + 1, widthResize, mResizeDistancesY[i].second},
            RECT{x1 + offsetX, y1 + offsetY, widthResize, resizeY}, painter);
        y1 = mResizeDistancesY[i].first + mResizeDistancesY[i].second;
        offsetY += resizeY - mResizeDistancesY[i].second;
    }
    y1 = mResizeDistancesY[mResizeDistancesY.size() - 1].first + mResizeDistancesY[mResizeDistancesY.size() - 1].second;
    heightResize = mImage->get_height() - y1 - 2;
    x1 = 0;
    offsetX = 0;
    for (int i = 0; i < mResizeDistancesX.size(); i++) {
        drawConstPart(RECT{x1 + 1, y1 + 1, mResizeDistancesX[i].first - x1, heightResize},
            RECT{x1 + offsetX, y1 + offsetY, mResizeDistancesX[i].first - x1, heightResize}, painter);
        x1 = mResizeDistancesX[i].first;
        resizeX = round((double)mResizeDistancesX[i].second * factorX);
        lostX += resizeX - ((double)mResizeDistancesX[i].second * factorX);
        if (fabs(lostX) >= 1.f) {
            if (lostX < 0) {  resizeX += 1;  lostX += 1.0; }
	    else { resizeX -= 1;  lostX += 1.0; }
        }
        drawScaledPart(RECT{x1 + 1, y1 + 1, mResizeDistancesX[i].second, heightResize},
            RECT{x1 + offsetX, y1 + offsetY, resizeX, heightResize}, painter);
        x1 = mResizeDistancesX[i].first + mResizeDistancesX[i].second;
        offsetX += resizeX - mResizeDistancesX[i].second;
    }
    x1 = mResizeDistancesX[mResizeDistancesX.size() - 1].first + mResizeDistancesX[mResizeDistancesX.size() - 1].second;
    widthResize = mImage->get_width() - x1 - 2;
    y1 = mResizeDistancesY[mResizeDistancesY.size() - 1].first + mResizeDistancesY[mResizeDistancesY.size() - 1].second;
    heightResize = mImage->get_height() - y1 - 2;
    drawConstPart(RECT{x1 + 1, y1 + 1, widthResize, heightResize},
         RECT{x1 + offsetX, y1 + offsetY, widthResize, heightResize}, painter);
}

}/*endof namespace*/
