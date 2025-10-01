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
#include <drawables/hwvectordrawable.h>
#include <drawables/hwvectordrawableutils.h>
#include <core/pathmeasure.h>
#include <drawables/hwpathparser.h>
#include <porting/cdlog.h>
#include <float.h>
namespace cdroid{
namespace hwui{
const int Tree::MAX_CACHED_BITMAP_SIZE = 2048;
void Path::dump() {
    LOGD("Path: %s has %zu points", mName.c_str(), mProperties.getData().points.size());
}

// Called from UI thread during the initial setup/theme change.
Path::Path(const char* pathStr, size_t strLength) {
    PathParser::ParseResult result;
    PathData data;
    PathParser::getPathDataFromAsciiString(&data, &result, pathStr, strLength);
    mStagingProperties.setData(data);
}

Path::Path(const Path& path) : Node(path) {
    mStagingProperties.syncProperties(path.mStagingProperties);
}

const Cairo::RefPtr<cdroid::Path> Path::getUpdatedPath(bool useStagingData, Cairo::RefPtr<cdroid::Path> tempStagingPath) {
    if (useStagingData) {
        tempStagingPath->reset();
        VectorDrawableUtils::verbsToPath(tempStagingPath, mStagingProperties.getData());
        return tempStagingPath;
    } else {
        if (mSkPathDirty) {
            mSkPath.reset();
            VectorDrawableUtils::verbsToPath(mSkPath, mProperties.getData());
            mSkPathDirty = false;
        }
        return mSkPath;
    }
}

void Path::syncProperties() {
    if (mStagingPropertiesDirty) {
        mProperties.syncProperties(mStagingProperties);
    } else {
        mStagingProperties.syncProperties(mProperties);
    }
    mStagingPropertiesDirty = false;
}


FullPath::FullPath(const FullPath& path) : Path(path) {
    mStagingProperties.syncProperties(path.mStagingProperties);
}

static void applyTrim(Cairo::RefPtr<cdroid::Path>& outPath, const Cairo::RefPtr<cdroid::Path> inPath, float trimPathStart, float trimPathEnd,
                      float trimPathOffset) {
    if (trimPathStart == 0.0f && trimPathEnd == 1.0f) {
        outPath = inPath;
        return;
    }
    outPath->reset();
    if (trimPathStart == trimPathEnd) {
        // Trimmed path should be empty.
        return;
    }
    PathMeasure measure(inPath,false);
    const float len = measure.getLength();
    const float start = len * fmod((trimPathStart + trimPathOffset), 1.0f);
    const float end = len * fmod((trimPathEnd + trimPathOffset), 1.0f);

    if (start > end) {
        measure.getSegment(start, len, outPath, true);
        if (end > 0) {
            measure.getSegment(0, end, outPath, true);
        }
    } else {
        measure.getSegment(start, end, outPath, true);
    }
}

const Cairo::RefPtr<cdroid::Path> FullPath::getUpdatedPath(bool useStagingData,Cairo::RefPtr<cdroid::Path> tempStagingPath) {
    if (!useStagingData && !mSkPathDirty && !mProperties.mTrimDirty) {
        return mTrimmedSkPath;
    }
    Path::getUpdatedPath(useStagingData, tempStagingPath);
    Cairo::RefPtr<cdroid::Path> outPath;

    if (useStagingData) {
        Cairo::RefPtr<cdroid::Path> inPath = std::make_shared<cdroid::Path>(*tempStagingPath);
        applyTrim(tempStagingPath, inPath, mStagingProperties.getTrimPathStart(),
                  mStagingProperties.getTrimPathEnd(), mStagingProperties.getTrimPathOffset());
        outPath = tempStagingPath;
    } else {
        if (mProperties.getTrimPathStart() != 0.0f || mProperties.getTrimPathEnd() != 1.0f) {
            mProperties.mTrimDirty = false;
            applyTrim(mTrimmedSkPath, mSkPath, mProperties.getTrimPathStart(),
                      mProperties.getTrimPathEnd(), mProperties.getTrimPathOffset());
            outPath = mTrimmedSkPath;
        } else {
            outPath = mSkPath;
        }
    }
    const FullPathProperties& properties = useStagingData ? mStagingProperties : mProperties;
    const bool setFillPath = properties.getFillGradient() != nullptr || properties.getFillColor() != 0;//SK_ColorTRANSPARENT;
    if (setFillPath) {
        //outPath->setFillType(static_cast<SkPathFillType>(properties.getFillType()));
    }
    return outPath;
}

void FullPath::dump() {
    Path::dump();
    LOGD("stroke width, color, alpha: %f, %d, %f, fill color, alpha: %d, %f",
          mProperties.getStrokeWidth(), mProperties.getStrokeColor(), mProperties.getStrokeAlpha(),
          mProperties.getFillColor(), mProperties.getFillAlpha());
}

static inline uint32_t applyAlpha(uint32_t color, float alpha) {
    color&=0x00FFFFFF;
    color|=(int(255*alpha)&0xFF)<<24;
    return color;
}

void FullPath::draw(Canvas& outCanvas, bool useStagingData) {
    Cairo::RefPtr<cdroid::Path> tempStagingPath = std::make_shared<cdroid::Path>();
    const FullPathProperties& properties = useStagingData ? mStagingProperties : mProperties;
    const Cairo::RefPtr<cdroid::Path> renderPath = getUpdatedPath(useStagingData, tempStagingPath);
    // Draw path's fill, if fill color or gradient is valid
    const uint32_t fillAlpha  = uint32_t(properties.getFillAlpha()*255.f)<<24;
    const uint32_t strokeAlpha= uint32_t(properties.getStrokeAlpha()*255.f)<<24;
    const bool needsFill  = (properties.getFillGradient() != nullptr) || fillAlpha;
    const bool needsStroke= (properties.getStrokeGradient()!=nullptr) || strokeAlpha;

    outCanvas.set_antialias(mAntiAlias?Cairo::ANTIALIAS_GRAY:Cairo::ANTIALIAS_NONE);
    renderPath->append_to_context(&outCanvas);
    if (needsFill) {
        if(properties.getFillGradient())
            outCanvas.set_source(properties.getFillGradient());
        else outCanvas.set_color(properties.getFillColor()|fillAlpha);
        outCanvas.set_fill_rule((Cairo::Context::FillRule)properties.getFillType());// EVEN_ODD WINDING
        if(needsStroke)
            outCanvas.fill_preserve();
        else outCanvas.fill();//drawPath(renderPath, paint);
    }

    if (needsStroke) {
        if(properties.getStrokeGradient())
            outCanvas.set_source(properties.getStrokeGradient());
        else outCanvas.set_color(properties.getStrokeColor()|strokeAlpha);
        outCanvas.set_line_join((Cairo::Context::LineJoin)properties.getStrokeLineJoin());
        //paint.setStrokeJoin(SkPaint::Join(properties.getStrokeLineJoin()));
        outCanvas.set_line_cap((Cairo::Context::LineCap)properties.getStrokeLineCap());
        //paint.setStrokeCap(SkPaint::Cap(properties.getStrokeLineCap()));
        outCanvas.set_miter_limit(properties.getStrokeMiterLimit());//paint.setStrokeMiter(properties.getStrokeMiterLimit());
        outCanvas.set_line_width(properties.getStrokeWidth());//paint.setStrokeWidth(properties.getStrokeWidth());
        outCanvas.stroke();//drawPath(renderPath, paint);
    }
}

void FullPath::syncProperties() {
    Path::syncProperties();

    if (mStagingPropertiesDirty) {
        mProperties.syncProperties(mStagingProperties);
    } else {
        // Update staging property with property values from animation.
        mStagingProperties.syncProperties(mProperties);
    }
    mStagingPropertiesDirty = false;
}


//REQUIRE_COMPATIBLE_LAYOUT(FullPath::FullPathProperties::PrimitiveFields);

static_assert(sizeof(float) == sizeof(int32_t), "float is not the same size as int32_t");
static_assert(sizeof(uint32_t) == sizeof(int32_t), "SkColor is not the same size as int32_t");

bool FullPath::FullPathProperties::copyProperties(int8_t* outProperties, int length) const {
    int propertyDataSize = sizeof(FullPathProperties::PrimitiveFields);
    if (length != propertyDataSize) {
        FATAL("Properties needs exactly %d bytes, a byte array of size %d is provided",
                         propertyDataSize, length);
        return false;
    }

    PrimitiveFields* out = reinterpret_cast<PrimitiveFields*>(outProperties);
    *out = mPrimitiveFields;
    return true;
}

void FullPath::FullPathProperties::setColorPropertyValue(int propertyId, int32_t value) {
    Property currentProperty = static_cast<Property>(propertyId);
    if (currentProperty == Property::strokeColor) {
        setStrokeColor(value);
    } else if (currentProperty == Property::fillColor) {
        setFillColor(value);
    } else {
        FATAL("Error setting color property on FullPath: No valid property with id: %d", propertyId);
    }
}

void FullPath::FullPathProperties::setPropertyValue(int propertyId, float value) {
    Property property = static_cast<Property>(propertyId);
    switch (property) {
    case Property::strokeWidth: setStrokeWidth(value); break;
    case Property::strokeAlpha: setStrokeAlpha(value); break;
    case Property::fillAlpha  : setFillAlpha(value); break;
    case Property::trimPathStart: setTrimPathStart(value); break;
    case Property::trimPathEnd: setTrimPathEnd(value); break;
    case Property::trimPathOffset:setTrimPathOffset(value); break;
    default: FATAL("Invalid property id: %d for animation", propertyId);break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void ClipPath::draw(Canvas& outCanvas, bool useStagingData) {
    Cairo::RefPtr<cdroid::Path> tempStagingPath;
    getUpdatedPath(useStagingData, tempStagingPath);
    tempStagingPath->append_to_context(&outCanvas);
    outCanvas.clip();
}

Group::Group(const Group& group) : Node(group) {
    mStagingProperties.syncProperties(group.mStagingProperties);
}

void Group::draw(Canvas& outCanvas, bool useStagingData) {
    // Save the current clip and matrix information, which is local to this group.
    // apply the current group's matrix to the canvas
    Cairo::Matrix stackedMatrix = Cairo::identity_matrix();
    const GroupProperties& prop = useStagingData ? mStagingProperties : mProperties;
    double scalex = prop.getScaleX();
    getLocalMatrix(stackedMatrix, prop);
    outCanvas.save();
    outCanvas.transform(stackedMatrix);
    // Draw the group tree in the same order as the XML file.
    for (auto& child : mChildren) {
        child->draw(outCanvas, useStagingData);
    }
    outCanvas.restore();
    // Restore the previous clip and matrix information.
}

void Group::dump() {
    LOGD("Group %s has %zu children: ", mName.c_str(), mChildren.size());
    LOGD("Group translateX, Y : %f, %f, scaleX, Y: %f, %f", mProperties.getTranslateX(),
          mProperties.getTranslateY(), mProperties.getScaleX(), mProperties.getScaleY());
    for (size_t i = 0; i < mChildren.size(); i++) {
        mChildren[i]->dump();
    }
}

void Group::syncProperties() {
    // Copy over the dirty staging properties
    if (mStagingPropertiesDirty) {
        mProperties.syncProperties(mStagingProperties);
    } else {
        mStagingProperties.syncProperties(mProperties);
    }
    mStagingPropertiesDirty = false;
    for (auto& child : mChildren) {
        child->syncProperties();
    }
}

static float limitScale(float scale){
    const float sign = (scale >= 0) ? 1.0f : -1.0f;
    return (std::abs(scale)<FLT_EPSILON)?sign*FLT_EPSILON:scale;
}

void Group::getLocalMatrix(Cairo::Matrix& outMatrix, const GroupProperties& properties) {
    outMatrix.translate(properties.getPivotX(), properties.getPivotY());
    outMatrix.scale(limitScale(properties.getScaleX()), limitScale(properties.getScaleY()));
    outMatrix.rotate(properties.getRotation() * M_PI / 180.0);
    outMatrix.translate(-properties.getPivotX(), -properties.getPivotY());

    Cairo::Matrix translate = Cairo::translation_matrix(properties.getTranslateX(),properties.getTranslateY());
    outMatrix.multiply(outMatrix,translate);
}

void Group::addChild(Node* child) {
    mChildren.emplace_back(child);
    if (mPropertyChangedListener != nullptr) {
        child->setPropertyChangedListener(mPropertyChangedListener);
    }
}

bool Group::GroupProperties::copyProperties(float* outProperties, int length) const {
    int propertyCount = static_cast<int>(Property::count);
    if (length != propertyCount) {
        FATAL("Properties needs exactly %d bytes, a byte array of size %d is provided",propertyCount, length);
        return false;
    }

    PrimitiveFields* out = reinterpret_cast<PrimitiveFields*>(outProperties);
    *out = mPrimitiveFields;
    return true;
}

// TODO: Consider animating the properties as float pointers
// Called on render thread
float Group::GroupProperties::getPropertyValue(int propertyId) const {
    Property currentProperty = static_cast<Property>(propertyId);
    switch (currentProperty) {
        case Property::rotate:  return getRotation();
        case Property::pivotX:  return getPivotX();
        case Property::pivotY:  return getPivotY();
        case Property::scaleX:  return getScaleX();
        case Property::scaleY:  return getScaleY();
        case Property::translateX:return getTranslateX();
        case Property::translateY:return getTranslateY();
        default:
            FATAL("Invalid property index: %d", propertyId);
            return 0;
    }
}

// Called on render thread
void Group::GroupProperties::setPropertyValue(int propertyId, float value) {
    Property currentProperty = static_cast<Property>(propertyId);
    switch (currentProperty) {
    case Property::rotate:  setRotation(value); break;
    case Property::pivotX:  setPivotX(value);   break;
    case Property::pivotY:  setPivotY(value);   break;
    case Property::scaleX:  setScaleX(value);   break;
    case Property::scaleY:  setScaleY(value);   break;
    case Property::translateX:  setTranslateX(value);   break;
    case Property::translateY:  setTranslateY(value);   break;
    default: FATAL("Invalid property index: %d", propertyId);
    }
}

bool Group::isValidProperty(int propertyId) {
    return GroupProperties::isValidProperty(propertyId);
}

bool Group::GroupProperties::isValidProperty(int propertyId) {
    return propertyId >= 0 && propertyId < static_cast<int>(Property::count);
}

int Tree::draw(Canvas& outCanvas, ColorFilter* colorFilter, const Rect& bounds, bool needsMirroring, bool canReuseCache) {
    // The imageView can scale the canvas in different ways, in order to
    // avoid blurry scaling, we have to draw into a bitmap with exact pixel
    // size first. This bitmap size is determined by the bounds and the
    // canvas scale.
    Cairo::Matrix canvasMatrix;
    outCanvas.get_matrix(canvasMatrix);
    float canvasScaleX = 1.0f;
    float canvasScaleY = 1.0f;
    if (canvasMatrix.xy==0/*getSkewX() == 0*/ && canvasMatrix.yx==0/*getSkewY() == 0*/) {
        // Only use the scale value when there's no skew or rotation in the canvas matrix.
        // TODO: Add a cts test for drawing VD on a canvas with negative scaling factors.
        canvasScaleX = fabs(canvasMatrix.xx);//getScaleX());
        canvasScaleY = fabs(canvasMatrix.yy);//getScaleY());
    }
    int scaledWidth = (int)(bounds.width * canvasScaleX);
    int scaledHeight = (int)(bounds.height * canvasScaleY);
    scaledWidth = std::min(Tree::MAX_CACHED_BITMAP_SIZE, scaledWidth);
    scaledHeight = std::min(Tree::MAX_CACHED_BITMAP_SIZE, scaledHeight);

    if (scaledWidth <= 0 || scaledHeight <= 0) {
        return 0;
    }

    mStagingProperties.setScaledSize(scaledWidth, scaledHeight);
    outCanvas.save();//int saveCount =outCanvas->save(SaveFlags::MatrixClip);
    outCanvas.translate(bounds.left, bounds.top);

    // Handle RTL mirroring.
    if (needsMirroring) {
        outCanvas.translate(bounds.width, 0);
        outCanvas.scale(-1.0f, 1.0f);
    }
    mStagingProperties.setColorFilter(colorFilter);

    // At this point, canvas has been translated to the right position.
    // And we use this bound for the destination rect for the drawBitmap, so
    // we offset to (0, 0);
    Rect tmpBounds = bounds;
    tmpBounds.left=0;tmpBounds.top= 0;
    mStagingProperties.setBounds(tmpBounds);
    drawStaging(outCanvas);
    outCanvas.restore();
    return scaledWidth * scaledHeight;
}

void Tree::drawStaging(Canvas& outCanvas) {
    bool redrawNeeded = allocateBitmapIfNeeded(mStagingCache, mStagingProperties.getScaledWidth(),
                                               mStagingProperties.getScaledHeight());
    // draw bitmap cache
    if (redrawNeeded || mStagingCache.dirty) {
        updateBitmapCache(mStagingCache.bitmap, true);
        mStagingCache.dirty = false;
    }

    /*SkPaint tmpPaint;
    SkPaint* paint = updatePaint(&tmpPaint, &mStagingProperties);
    outCanvas.drawBitmap(*mStagingCache.bitmap, 0, 0, mStagingCache.bitmap->get_width(),
                          mStagingCache.bitmap->get_height(), mStagingProperties.getBounds().left,
                          mStagingProperties.getBounds().top,
                          mStagingProperties.getBounds().right(),
                          mStagingProperties.getBounds().bottom(), paint);*/
    outCanvas.set_source(mStagingCache.bitmap,0,0);
    outCanvas.set_operator(Cairo::Context::Operator::SOURCE);
    outCanvas.scale(float(mStagingProperties.getBounds().width)/mStagingCache.bitmap->get_width(),
            float(mStagingProperties.getBounds().height)/mStagingCache.bitmap->get_height());
    outCanvas.paint();
}

void Tree::updatePaint(Tree::TreeProperties*prop,Cairo::RefPtr<Cairo::Pattern>&stroke,Cairo::RefPtr<Cairo::Pattern>&fill){
    /*if (prop->getRootAlpha() == 1.0f && prop->getColorFilter() == nullptr) {
        return nullptr;
    } else {
        outPaint->setColorFilter(sk_ref_sp(prop->getColorFilter()));
        outPaint->setFilterQuality(kLow_SkFilterQuality);
        outPaint->setAlpha(prop->getRootAlpha() * 255);
        return outPaint;
    }*/
}

void Tree::updateBitmapCache(Bitmap& bitmap, bool useStagingData) {
    Bitmap outCache = bitmap;
    const int cacheWidth = outCache->get_width();
    const int cacheHeight = outCache->get_height();
    Canvas outCanvas(outCache);
    const auto op = outCanvas.get_operator();
    outCanvas.set_operator(Cairo::Context::Operator::CLEAR);
    outCanvas.set_source_rgba(0,0,0,0);
    outCanvas.paint();
    outCanvas.set_operator(op);
    const float viewportWidth = useStagingData ? mStagingProperties.getViewportWidth() : mProperties.getViewportWidth();
    const float viewportHeight= useStagingData ? mStagingProperties.getViewportHeight() : mProperties.getViewportHeight();
    const float scaleX = cacheWidth / viewportWidth;
    const float scaleY = cacheHeight / viewportHeight;
    outCanvas.scale(scaleX, scaleY);
    mRootNode->draw(outCanvas, useStagingData);
}

bool Tree::allocateBitmapIfNeeded(Cache& cache, int width, int height) {
    if (!canReuseBitmap(cache.bitmap, width, height)) {
        cache.bitmap = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,width, height);//Bitmap::allocateHeapBitmap(info);
        return true;
    }
    return false;
}

bool Tree::canReuseBitmap(Bitmap& bitmap, int width, int height)const {
    return bitmap && (width <= bitmap->get_width()) && (height <= bitmap->get_height());
}

void Tree::onPropertyChanged(TreeProperties* prop) {
    if (prop == &mStagingProperties) {
        mStagingCache.dirty = true;
    } else {
        mCache.dirty = true;
    }
}

}/*endof namespace vectordrawable*/
}/*endof namespace cdroid*/
