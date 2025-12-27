/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <drawable/ninepatch.h>
#include <drawable/ninepatchrenderer.h>
#include <core/context.h>
#include <image-decoders/imagedecoder.h>
#include <porting/cdlog.h>
/*
 * REFS:
 * frameworks/base/libs/hwui/jni/NinePatchPeeker.cpp
 * frameworks/base/libs/androidfw/Png.cpp
 */
using namespace Cairo;

namespace cdroid{
constexpr uint32_t kColorWhite = 0xffffffffu;
constexpr uint32_t kColorTick = 0xff000000u;
constexpr uint32_t kColorLayoutBoundsTick = 0xff0000ffu;
enum class TickType { kNone, kTick, kLayoutBounds, kBoth };

static TickType tickType(uint8_t* p, bool transparent, const char** outError) {
    uint32_t color = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);

    if (transparent) {
        if (p[3] == 0) {
            return TickType::kNone;
        }
        if (color == kColorLayoutBoundsTick) {
            return TickType::kLayoutBounds;
        }
        if (color == kColorTick) {
            return TickType::kTick;
        }

        // Error cases
        if (p[3] != 0xff) {
            *outError ="Frame pixels must be either solid or transparent (not intermediate alphas)";
            return TickType::kNone;
        }

        if (p[0] != 0 || p[1] != 0 || p[2] != 0) {
            *outError = "Ticks in transparent frame must be black or red";
        }
        return TickType::kTick;
    }

    if (p[3] != 0xFF) {
        *outError = "White frame must be a solid color (no alpha)";
    }
    if (color == kColorWhite) {
        return TickType::kNone;
    }
    if (color == kColorTick) {
        return TickType::kTick;
    }
    if (color == kColorLayoutBoundsTick) {
        return TickType::kLayoutBounds;
    }

    if (p[0] != 0 || p[1] != 0 || p[2] != 0) {
        *outError = "Ticks in white frame must be black or red";
        return TickType::kNone;
    }
    return TickType::kTick;
}

enum class TickState { kStart, kInside1, kOutside1 };

static bool getHorizontalTicks(uint8_t* row, int width, bool transparent, bool required,
                               int32_t* outLeft, int32_t* outRight, const char** outError,
                               uint8_t* outDivs, bool multipleAllowed) {
    *outLeft = *outRight = -1;
    TickState state = TickState::kStart;
    bool found = false;

    for (int i = 1; i < width - 1; i++) {
        if (tickType(row + i * 4, transparent, outError) == TickType::kTick) {
            if (state == TickState::kStart || (state == TickState::kOutside1 && multipleAllowed)) {
                *outLeft = i - 1;
                *outRight = width - 2;
                found = true;
                if (outDivs != NULL) {
                    *outDivs += 2;
                }
                state = TickState::kInside1;
            } else if (state == TickState::kOutside1) {
                *outError = "Can't have more than one marked region along edge";
                *outLeft = i;
                return false;
            }
        } else if (!*outError) {
            if (state == TickState::kInside1) {
                // We're done with this div.  Move on to the next.
                *outRight = i - 1;
                outRight += 2;
                outLeft += 2;
                state = TickState::kOutside1;
            }
        } else {
            *outLeft = i;
            return false;
        }
    }

    if (required && !found) {
        *outError = "No marked region found along edge";
        *outLeft = -1;
        return false;
    }
    return true;
}

static bool getVerticalTicks(uint8_t** rows, int offset, int height, bool transparent, bool required,
        int32_t* outTop, int32_t* outBottom, const char** outError, uint8_t* outDivs, bool multipleAllowed) {
    *outTop = *outBottom = -1;
    TickState state = TickState::kStart;
    bool found = false;

    for (int i = 1; i < height - 1; i++) {
        if (tickType(rows[i] + offset, transparent, outError) == TickType::kTick) {
            if (state == TickState::kStart || (state == TickState::kOutside1 && multipleAllowed)) {
                *outTop = i - 1;
                *outBottom = height - 2;
                found = true;
                if (outDivs != NULL) {
                    *outDivs += 2;
                }
                state = TickState::kInside1;
            } else if (state == TickState::kOutside1) {
                *outError = "Can't have more than one marked region along edge";
                *outTop = i;
                return false;
            }
        } else if (!*outError) {
            if (state == TickState::kInside1) {
                // We're done with this div.  Move on to the next.
                *outBottom = i - 1;
                outTop += 2;
                outBottom += 2;
                state = TickState::kOutside1;
            }
        } else {
            *outTop = i;
            return false;
        }
    }

    if (required && !found) {
        *outError = "No marked region found along edge";
        *outTop = -1;
        return false;
    }
    return true;
}

static bool getHorizontalLayoutBoundsTicks(uint8_t* row, int width, bool transparent,
                bool /* required */, int32_t* outLeft, int32_t* outRight,const char** outError) {
    *outLeft = *outRight = 0;

    // Look for left tick
    if (tickType(row + 4, transparent, outError) == TickType::kLayoutBounds) {
        // Starting with a layout padding tick
        int i = 1;
        while (i < width - 1) {
            (*outLeft)++;
            i++;
            if (tickType(row + i * 4, transparent, outError) != TickType::kLayoutBounds) {
                break;
            }
        }
    }

    // Look for right tick
    if (tickType(row + (width - 2) * 4, transparent, outError) == TickType::kLayoutBounds) {
        // Ending with a layout padding tick
        int i = width - 2;
        while (i > 1) {
            (*outRight)++;
            i--;
            if (tickType(row + i * 4, transparent, outError) != TickType::kLayoutBounds) {
                break;
            }
        }
    }
    return true;
}

static bool getVerticalLayoutBoundsTicks(uint8_t** rows, int offset, int height, bool transparent,
                                         bool /* required */, int32_t* outTop, int32_t* outBottom,
                                         const char** outError) {
    *outTop = *outBottom = 0;

    // Look for top tick
    if (tickType(rows[1] + offset, transparent, outError) == TickType::kLayoutBounds) {
        // Starting with a layout padding tick
        int i = 1;
        while (i < height - 1) {
            (*outTop)++;
            i++;
            if (tickType(rows[i] + offset, transparent, outError) != TickType::kLayoutBounds) {
                break;
            }
        }
    }

    // Look for bottom tick
    if (tickType(rows[height - 2] + offset, transparent, outError) == TickType::kLayoutBounds) {
        // Ending with a layout padding tick
        int i = height - 2;
        while (i > 1) {
            (*outBottom)++;
            i--;
            if (tickType(rows[i] + offset, transparent, outError) != TickType::kLayoutBounds) {
                break;
            }
        }
    }
    return true;
}

static void findMaxOpacity(uint8_t** rows, int startX, int startY, int endX, int endY, int dX,
        int dY, int* outInset) {
    uint8_t maxOpacity = 0;
    int inset = 0;
    *outInset = 0;
    for (int x = startX, y = startY; x != endX && y != endY; x += dX, y += dY, inset++) {
        uint8_t* color = rows[y] + x * 4;
        uint8_t opacity = color[3];
        if (opacity > maxOpacity) {
            maxOpacity = opacity;
            *outInset = inset;
        }
        if (opacity == 0xff) return;
    }
}


static uint8_t maxAlphaOverRow(uint8_t* row, int startX, int endX) {
    uint8_t maxAlpha = 0;
    for (int x = startX; x < endX; x++) {
        uint8_t alpha = (row + x * 4)[3];
        if (alpha > maxAlpha) maxAlpha = alpha;
    }
    return maxAlpha;
}

static uint8_t maxAlphaOverCol(uint8_t** rows, int offsetX, int startY, int endY) {
    uint8_t maxAlpha = 0;
    for (int y = startY; y < endY; y++) {
        uint8_t alpha = (rows[y] + offsetX * 4)[3];
        if (alpha > maxAlpha) maxAlpha = alpha;
    }
    return maxAlpha;
}

NinePatchRenderer::NinePatchRenderer(Cairo::RefPtr<ImageSurface> image)
    : mImage(image){
    const uint32_t numRows = image->get_height();
    auto rows = std::unique_ptr<uint8_t*[]>(new uint8_t*[numRows]);
    uint8_t*pd = image->get_data();
    for(int i = 0;i < numRows; i++){
        rows[i] = pd;
        pd += image->get_stride();
    }
    auto anp = NinePatch::Create(rows.get(),image->get_width(),numRows,nullptr);
    for(auto&r:anp->horizontal_stretch_regions){
        mResizeDistancesX.push_back({r.start,r.end-r.start});
    }
    for(auto&r:anp->vertical_stretch_regions){
        mResizeDistancesY.push_back({r.start,r.end-r.start});
    }

    const int midX = image->get_width() / 2;
    const int midY = image->get_height() / 2;
    const int endX = image->get_width() - 2;
    const int endY = image->get_height() - 2;

   // find left and right extent of nine patch content on center row
    if (image->get_width() > 4) {
        findMaxOpacity(rows.get(), 1, midY, midX, -1, 1, 0, &mOutlineInsets.left);
        findMaxOpacity(rows.get(), endX, midY, midX, -1, -1, 0, &mOutlineInsets.bottom);
    } else {
        mOutlineInsets.left = 0;
        mOutlineInsets.right = 0;
    }
 
    // find top and bottom extent of nine patch content on center column
    if (image->get_height() > 4) {
        findMaxOpacity(rows.get(), midX, 1, -1, midY, 0, 1, &mOutlineInsets.top);
        findMaxOpacity(rows.get(), midX, endY, -1, midY, 0, -1, &mOutlineInsets.bottom);
    } else {
        mOutlineInsets.top = 0;
        mOutlineInsets.bottom = 0;
    }

    
    // Find left and right of layout padding...(maybe its is android's OpticalInsets)
    const bool transparent = rows[0][3]==0;
    const char* errorMsg=nullptr;
    getHorizontalLayoutBoundsTicks(rows[image->get_height() - 1], image->get_width(), transparent, false,
                                   &mOpticalInsets.left, &mOpticalInsets.right, &errorMsg);
  
    getVerticalLayoutBoundsTicks(rows.get(), (image->get_width() - 1) * 4, image->get_height(), transparent, false,
                                 &mOpticalInsets.top, &mOpticalInsets.bottom, &errorMsg);
  
    const bool haveLayoutBounds = (mOpticalInsets.left != 0) || (mOpticalInsets.right != 0)
        ||(mOpticalInsets.top != 0) || (mOpticalInsets.bottom != 0);

    const int innerStartX = 1 + mOutlineInsets.left;
    const int innerStartY = 1 + mOutlineInsets.top;
    const int innerEndX = endX - mOutlineInsets.right;
    const int innerEndY = endY - mOutlineInsets.bottom;
    const int innerMidX = (innerEndX + innerStartX) / 2;
    const int innerMidY = (innerEndY + innerStartY) / 2;
  
    // assuming the image is a round rect, compute the radius by marching
    // diagonally from the top left corner towards the center
    mAlpha = std::max(maxAlphaOverRow(rows[innerMidY], innerStartX, innerEndX),
                 maxAlphaOverCol(rows.get(), innerMidX, innerStartY, innerStartY));
  
    int diagonalInset = 0;
    findMaxOpacity(rows.get(), innerStartX, innerStartY, innerMidX, innerMidY, 1, 1, &diagonalInset);
  
    /* Determine source radius based upon inset:
     *     sqrt(r^2 + r^2) = sqrt(i^2 + i^2) + r
     *     sqrt(2) * r = sqrt(2) * i + r
     *     (sqrt(2) - 1) * r = sqrt(2) * i
     *     r = sqrt(2) / (sqrt(2) - 1) * i
     */
    mRadius = int32_t(3.4142f * diagonalInset);

    mPadding.left = anp->padding.left;
    mPadding.top  = anp->padding.top;
    mPadding.width = anp->padding.right;
    mPadding.height= anp->padding.bottom;
    mRadius = static_cast<int>(anp->outline_radius);
    mOpacity = ImageDecoder::getTransparency(mImage);
    mContentArea.set(mPadding.left,mPadding.top,
        image->get_width()-mPadding.left-2,
        image->get_height()-mPadding.top-2);
    mAlpha =1.f;
    if (!mResizeDistancesX.size() || !mResizeDistancesY.size()) {
        //throw new ExceptionNot9Patch;
        throw "Not ninepatch image!";
    }
    LOGD_IF(haveLayoutBounds,"OutlineInsets=(%d,%d,%d,%d) OpticalInsets=(%d,%d,%d,%d) padding=(%d,%d,%d,%d)",
            mOpticalInsets.left,mOpticalInsets.top,mOpticalInsets.right,mOpticalInsets.bottom,
            mOpticalInsets.left,mOpticalInsets.top,mOpticalInsets.right,mOpticalInsets.bottom,
            mPadding.left,mPadding.top,mPadding.width,mPadding.height);
}

NinePatchRenderer::NinePatchRenderer(Context*ctx,const std::string&resid)
    :NinePatchRenderer(ImageDecoder::loadImage(ctx,resid)){
}

NinePatchRenderer::~NinePatchRenderer() {
}

void NinePatchRenderer::draw(Canvas& painter, int  x, int  y,float alpha) {
    Cairo::Matrix ctx = painter.get_matrix();
    const double radians = atan2(ctx.yy, ctx.xy);
    const int rotDegrees = int(radians*180.f/M_PI)%90;
    const bool isScaling = (ctx.xx!=1.f)||(ctx.yy!=1.f);
    painter.save();
    painter.translate(x,y);
    mAlpha = alpha;
    const SurfacePattern::Filter filterMode = (rotDegrees||isScaling)?SurfacePattern::Filter::BILINEAR:SurfacePattern::Filter::NEAREST;
    painter.set_source(mCachedImage,0,0);
    painter.rectangle(0,0,mCachedImage->get_width(),mCachedImage->get_height());
    painter.clip();
    Cairo::RefPtr<SurfacePattern>spat = painter.get_source_for_surface();
    if(spat)spat->set_filter(filterMode);
    painter.paint();
    painter.restore();
}

void NinePatchRenderer::draw(Canvas& painter, const Rect&rect,float alpha){
    int resizeWidth = 0,resizeHeight = 0;
    const int width = rect.width;
    const int height= rect.height;
    std::ostringstream oss;

    painter.save();
    painter.translate(rect.left,rect.top);
    mAlpha = alpha;
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
    LOGE_IF(hasErrors,"%s",oss.str().c_str());
    mWidth = rect.width;
    mHeight= rect.height;
	painter.save();
	painter.translate(rect.left,rect.top);
    updateCachedImage(mWidth,mHeight,&painter);
	painter.restore();
}

void NinePatchRenderer::setImageSize(int width, int height) {
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
    if(oss.str().empty()==false){
        LOG(ERROR)<<oss.str();
    }
    if (width != mWidth || height != mHeight) {
        mWidth = width;
        mHeight = height;
        updateCachedImage(width, height,nullptr);
    }
}

Rect NinePatchRenderer::getPadding()const{
    return mPadding;
}

Insets NinePatchRenderer::getOpticalInsets()const{
    return mOpticalInsets;
}

int NinePatchRenderer::getRadius()const{
    return mRadius;
}

void NinePatchRenderer::drawScaledPart(const Rect& oldRect, const Rect& newRect,Cairo::Context&painter) {
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

void NinePatchRenderer::drawConstPart(const Rect& oldRect, const Rect& newRect,Cairo::Context&painter) {
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
    const unsigned char*data=img->get_data()+img->get_stride()*j+i*4;
    const uint8_t r = data[0];
    const uint8_t g = data[1];
    const uint8_t b = data[2];
    const uint8_t a = data[3];
    if (a < 128) return false;
    return (r < 128 && g < 128 && b < 128);
}

int NinePatchRenderer::getCornerRadius(Cairo::RefPtr<ImageSurface> bitmap,int start,int step) {
    const int width = bitmap->get_width();
    const int height = bitmap->get_height();
    int cornerRadius = 0;
    const int end=std::min(width, height);
    for (int i = start; i < end; i+=step) {
        uint32_t* pixel = (uint32_t*)(bitmap->get_data()+bitmap->get_stride()*i+ i*4);
        if (*pixel != 0) {
            break;
        }
        cornerRadius++;
    }
    return cornerRadius;
}

Insets NinePatchRenderer::getOpticalInsets(Cairo::RefPtr<ImageSurface>bitmap) const{
    Insets insets;
    const int width = bitmap->get_width();
    const int height= bitmap->get_height();
    const int stride= bitmap->get_stride();
    uint8_t* data = bitmap->get_data();
    for (int x = 1; x < width-1; ++x) {/*LEFT*/
        bool opaqueFound = false;
        for (int y = 1; y < height-1; ++y) {
            uint32_t* pixel = reinterpret_cast<uint32_t*>(data + y * stride + x * 4);
            uint8_t alpha = (*pixel >> 24) & 0xFF;
            if (alpha != 0) {
                opaqueFound = true;
                break;
            }
        }
        if (opaqueFound) {
            insets.left = x-1;
            break;
        }
    }

    for (int x = width - 2; x >= 1; --x) {/*RIGHT*/
        bool opaqueFound = false;
        for (int y = 0; y < height; ++y) {
            uint32_t* pixel = reinterpret_cast<uint32_t*>(data + y * stride + x * 4);
            uint8_t alpha = (*pixel >> 24) & 0xFF;
            if (alpha != 0) {
                opaqueFound = true;
                break;
            }
        }
        if (opaqueFound) {
            insets.right = width - 2 - x;
            break;
        }
    }

    for (int y = 1; y < height-1; ++y) {/*TOP*/
        bool opaqueFound = false;
        for (int x = 1; x < width-1; ++x) {
            uint32_t* pixel = reinterpret_cast<uint32_t*>(data + y * stride + x * 4);
            uint8_t alpha = (*pixel >> 24) & 0xFF;
            if (alpha != 0) {
                opaqueFound = true;
                break;
            }
        }
        if (opaqueFound) {
            insets.top = y-1;
            break;
        }
    }

    for (int y = height - 2; y >= 1; --y) {/*BOTTOM*/
        bool opaqueFound = false;
        for (int x = 0; x < width; ++x) {
            uint32_t* pixel = reinterpret_cast<uint32_t*>(data + y * stride + x * 4);
            uint8_t alpha = (*pixel >> 24) & 0xFF;
            if (alpha != 0) {
                opaqueFound = true;
                break;
            }
        }
        if (opaqueFound) {
            insets.bottom = height - 2 - y;
            break;
        }
    }
    return insets;
}

void NinePatchRenderer::getFactor(int width, int height, double& factorX, double& factorY) {
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

void NinePatchRenderer::updateCachedImage(int width, int height,Cairo::Context*painterIn) {
    double lostX  = 0.f, lostY  = 0.f;
    double factorX= 0.f, factorY= 0.f;
    int x1 = 0 , y1 = 0; //for image parts X/Y
    int widthResize,heightResize; //width/height for image parts
    int resizeX = 0 , resizeY ;
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

Rect NinePatchRenderer::getOutlineRect() const {
    // 如果有专用的 mOutlineRect，优先返回
    // return mOutlineRect;
    // 否则用内容区或 padding 作为 fallback
    if (!mContentArea.empty())
        return mContentArea;
    else
        return getPadding();
}

int NinePatchRenderer::getOutlineRadius() const {
    return mRadius;
}
}/*endof namespace*/
