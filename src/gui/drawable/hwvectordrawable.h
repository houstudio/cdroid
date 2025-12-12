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
#ifndef __HWUI_VECTOR_DRAWABLE_H__
#define __HWUI_VECTOR_DRAWABLE_H__
#include <vector>
#include <functional>
#include <cairomm/context.h>
#include <core/canvas.h>
#include <core/path.h>
#include <drawable/colorfilters.h>
namespace cdroid{
namespace hwui{
#define VD_SET_PRIMITIVE_FIELD_WITH_FLAG(field, value, flag) \
    (VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(field, (value)) ? ((flag) = true, true) : false)
#define VD_SET_PROP(field, value) ((value) != (field) ? ((field) = (value), true) : false)
#define VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(field, value)               \
    ({                                                                \
        bool retVal = VD_SET_PROP((mPrimitiveFields.field), (value)); \
        onPropertyChanged();                                          \
        retVal;                                                       \
    })

/* A VectorDrawable is composed of a tree of nodes.
 * Each node can be a group node, or a path.
 * A group node can have groups or paths as children, but a path node has
 * no children.
 * One example can be:
 *                 Root Group
 *                /    |     \
 *           Group    Path    Group
 *          /     \             |
 *         Path   Path         Path
 *
 * VectorDrawables are drawn into bitmap caches first, then the caches are drawn to the given
 * canvas with root alpha applied. Two caches are maintained for VD, one in UI thread, the other in
 * Render Thread. A generation id is used to keep track of changes in the vector drawable tree.
 * Each cache has their own generation id to track whether they are up to date with the latest
 * change in the tree.
 *
 * Any property change to the vector drawable coming from UI thread (such as bulk setters to update
 * all the properties, and viewport change, etc.) are only modifying the staging properties. The
 * staging properties will then be marked dirty and will be pushed over to render thread properties
 * at sync point. If staging properties are not dirty at sync point, we sync backwards by updating
 * staging properties with render thread properties to reflect the latest animation value.
 *
 */

class PropertyChangedListener {
public:
    PropertyChangedListener(bool* dirty, bool* stagingDirty)
            : mDirty(dirty), mStagingDirty(stagingDirty) {}
    void onPropertyChanged() { *mDirty = true; }
    void onStagingPropertyChanged() { *mStagingDirty = true; }

private:
    bool* mDirty;
    bool* mStagingDirty;
};

class Node {
public:
    class Properties {
    public:
        explicit Properties(Node* node) : mNode(node) {}
        inline void onPropertyChanged() { mNode->onPropertyChanged(this); }

    private:
        Node* mNode;
    };
    Node(const Node& node) { mName = node.mName; }
    Node() {}
    virtual void draw(Canvas& outCanvas, bool useStagingData) = 0;
    virtual void dump() = 0;
    void setName(const char* name) { mName = name; }
    virtual void setPropertyChangedListener(PropertyChangedListener* listener) {
        mPropertyChangedListener = listener;
    }
    virtual void onPropertyChanged(Properties* properties) = 0;
    virtual ~Node() {}
    virtual void syncProperties() = 0;
    virtual void setAntiAlias(bool aa) = 0;

    virtual void forEachFillColor(const std::function<void(uint32_t)>& func) const { }

protected:
    std::string mName;
    PropertyChangedListener* mPropertyChangedListener = nullptr;
};

struct PathData {
    std::vector<char> verbs;
    std::vector<size_t> verbSizes;
    std::vector<float> points;
    bool operator==(const PathData& data) const {
        return verbs == data.verbs && verbSizes == data.verbSizes && points == data.points;
    }
};

class Path : public Node {
public:
    class PathProperties : public Properties {
    public:
        explicit PathProperties(Node* node) : Properties(node) {}
        void syncProperties(const PathProperties& prop) {
            mData = prop.mData;
            onPropertyChanged();
        }
        void setData(const PathData& data) {
            // Updates the path data. Note that we don't generate a new Skia path right away
            // because there are cases where the animation is changing the path data, but the view
            // that hosts the VD has gone off screen, in which case we won't even draw. So we
            // postpone the Skia path generation to the draw time.
            if (data == mData) {
                return;
            }
            mData = data;
            onPropertyChanged();
        }
        const PathData& getData() const { return mData; }

    private:
        PathData mData;
    };

    Path(const Path& path);
    Path(const char* path, size_t strLength);
    Path() {}

    void dump() override;
    virtual void syncProperties() override;
    virtual void onPropertyChanged(Properties* prop) override {
        if (prop == &mStagingProperties) {
            mStagingPropertiesDirty = true;
            if (mPropertyChangedListener) {
                mPropertyChangedListener->onStagingPropertyChanged();
            }
        } else if (prop == &mProperties) {
            mSkPathDirty = true;
            if (mPropertyChangedListener) {
                mPropertyChangedListener->onPropertyChanged();
            }
        }
    }
    PathProperties* mutateStagingProperties() { return &mStagingProperties; }
    const PathProperties* stagingProperties() { return &mStagingProperties; }

    // This should only be called from animations on RT
    PathProperties* mutateProperties() { return &mProperties; }

protected:
    virtual const Cairo::RefPtr<cdroid::Path> getUpdatedPath(bool useStagingData,Cairo::RefPtr<cdroid::Path> tempStagingPath);

    // Internal data, render thread only.
    bool mSkPathDirty = true;
    Cairo::RefPtr<cdroid::Path> mSkPath;
private:
    PathProperties mProperties = PathProperties(this);
    PathProperties mStagingProperties = PathProperties(this);
    bool mStagingPropertiesDirty = true;
};

class FullPath : public Path {
public:
    class FullPathProperties : public Properties {
    public:
        struct PrimitiveFields {
            float strokeWidth = 0;
            uint32_t strokeColor = 0;//SK_ColorTRANSPARENT;
            float strokeAlpha = 1;
            uint32_t fillColor = 0;//SK_ColorTRANSPARENT;
            float fillAlpha = 1;
            float trimPathStart = 0;
            float trimPathEnd = 1;
            float trimPathOffset = 0;
            int32_t strokeLineCap = 0;//SkPaint::Cap::kButt_Cap;
            int32_t strokeLineJoin = 0;//SkPaint::Join::kMiter_Join;
            float strokeMiterLimit = 4;
            int fillType = 0; /* non-zero or kWinding_FillType in Skia */
        };
        explicit FullPathProperties(Node* mNode) : Properties(mNode), mTrimDirty(false) {}
        ~FullPathProperties() {}
        void syncProperties(const FullPathProperties& prop) {
            mPrimitiveFields = prop.mPrimitiveFields;
            mTrimDirty = true;
            fillGradient = prop.fillGradient;
            strokeGradient = prop.strokeGradient;
            onPropertyChanged();
        }
        void setFillGradient(Cairo::RefPtr<Cairo::Pattern> gradient) {
            if (fillGradient != gradient) {
                fillGradient = gradient;
                onPropertyChanged();
            }
        }
        void setStrokeGradient(Cairo::RefPtr<Cairo::Pattern> gradient) {
            if (strokeGradient!= gradient) {
                strokeGradient = gradient;
                onPropertyChanged();
            }
        }
        Cairo::RefPtr<Cairo::Pattern> getFillGradient() const { return fillGradient; }
        Cairo::RefPtr<Cairo::Pattern> getStrokeGradient() const { return strokeGradient; }
        float getStrokeWidth() const { return mPrimitiveFields.strokeWidth; }
        void setStrokeWidth(float strokeWidth) {
            VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(strokeWidth, strokeWidth);
        }
        uint32_t getStrokeColor() const { return mPrimitiveFields.strokeColor; }
        void setStrokeColor(uint32_t strokeColor) {
            VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(strokeColor, strokeColor);
        }
        float getStrokeAlpha() const { return mPrimitiveFields.strokeAlpha; }
        void setStrokeAlpha(float strokeAlpha) {
            VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(strokeAlpha, strokeAlpha);
        }
        uint32_t getFillColor() const { return mPrimitiveFields.fillColor; }
        void setFillColor(uint32_t fillColor) {
            VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(fillColor, fillColor);
        }
        float getFillAlpha() const { return mPrimitiveFields.fillAlpha; }
        void setFillAlpha(float fillAlpha) {
            VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(fillAlpha, fillAlpha);
        }
        float getTrimPathStart() const { return mPrimitiveFields.trimPathStart; }
        void setTrimPathStart(float trimPathStart) {
            VD_SET_PRIMITIVE_FIELD_WITH_FLAG(trimPathStart, trimPathStart, mTrimDirty);
        }
        float getTrimPathEnd() const { return mPrimitiveFields.trimPathEnd; }
        void setTrimPathEnd(float trimPathEnd) {
            VD_SET_PRIMITIVE_FIELD_WITH_FLAG(trimPathEnd, trimPathEnd, mTrimDirty);
        }
        float getTrimPathOffset() const { return mPrimitiveFields.trimPathOffset; }
        void setTrimPathOffset(float trimPathOffset) {
            VD_SET_PRIMITIVE_FIELD_WITH_FLAG(trimPathOffset, trimPathOffset, mTrimDirty);
        }

        float getStrokeMiterLimit() const { return mPrimitiveFields.strokeMiterLimit; }
        int32_t getStrokeLineCap() const { return mPrimitiveFields.strokeLineCap; }
        int32_t getStrokeLineJoin() const { return mPrimitiveFields.strokeLineJoin; }
        int32_t getFillType() const { return mPrimitiveFields.fillType; }
        bool copyProperties(int8_t* outProperties, int length) const;
        void updateProperties(float strokeWidth, uint32_t strokeColor, float strokeAlpha,
                              uint32_t fillColor, float fillAlpha, float trimPathStart,
                              float trimPathEnd, float trimPathOffset, float strokeMiterLimit,
                              int strokeLineCap, int strokeLineJoin, int fillType) {
            mPrimitiveFields.strokeWidth = strokeWidth;
            mPrimitiveFields.strokeColor = strokeColor;
            mPrimitiveFields.strokeAlpha = strokeAlpha;
            mPrimitiveFields.fillColor = fillColor;
            mPrimitiveFields.fillAlpha = fillAlpha;
            mPrimitiveFields.trimPathStart = trimPathStart;
            mPrimitiveFields.trimPathEnd = trimPathEnd;
            mPrimitiveFields.trimPathOffset = trimPathOffset;
            mPrimitiveFields.strokeMiterLimit = strokeMiterLimit;
            mPrimitiveFields.strokeLineCap = strokeLineCap;
            mPrimitiveFields.strokeLineJoin = strokeLineJoin;
            mPrimitiveFields.fillType = fillType;
            mTrimDirty = true;
            onPropertyChanged();
        }
        // Set property values during animation
        void setColorPropertyValue(int propertyId, int32_t value);
        void setPropertyValue(int propertyId, float value);
        bool mTrimDirty;

    private:
        enum class Property {
            strokeWidth = 0,
            strokeColor,
            strokeAlpha,
            fillColor,
            fillAlpha,
            trimPathStart,
            trimPathEnd,
            trimPathOffset,
            strokeLineCap,
            strokeLineJoin,
            strokeMiterLimit,
            fillType,
            count,
        };
        PrimitiveFields mPrimitiveFields;
        Cairo::RefPtr<Cairo::Pattern> fillGradient;
        Cairo::RefPtr<Cairo::Pattern> strokeGradient;
    };

    // Called from UI thread
    FullPath(const FullPath& path);  // for cloning
    FullPath(const char* path, size_t strLength) : Path(path, strLength) {}
    FullPath() : Path() {}
    void draw(Canvas& outCanvas, bool useStagingData) override;
    void dump() override;
    FullPathProperties* mutateStagingProperties() { return &mStagingProperties; }
    const FullPathProperties* stagingProperties() { return &mStagingProperties; }

    // This should only be called from animations on RT
    FullPathProperties* mutateProperties() { return &mProperties; }

    virtual void syncProperties() override;
    virtual void onPropertyChanged(Properties* properties) override {
        Path::onPropertyChanged(properties);
        if (properties == &mStagingProperties) {
            mStagingPropertiesDirty = true;
            if (mPropertyChangedListener) {
                mPropertyChangedListener->onStagingPropertyChanged();
            }
        } else if (properties == &mProperties) {
            if (mPropertyChangedListener) {
                mPropertyChangedListener->onPropertyChanged();
            }
        }
    }
    void setAntiAlias(bool aa) override{ mAntiAlias = aa; }
    void forEachFillColor(const std::function<void(uint32_t)>& func) const override {
        func(mStagingProperties.getFillColor());
    }

protected:
    const Cairo::RefPtr<cdroid::Path> getUpdatedPath(bool useStagingData,Cairo::RefPtr<cdroid::Path> tempStagingPath) override;
private:
    FullPathProperties mProperties = FullPathProperties(this);
    FullPathProperties mStagingProperties = FullPathProperties(this);
    bool mStagingPropertiesDirty = true;

    // Intermediate data for drawing, render thread only
    Cairo::RefPtr<cdroid::Path> mTrimmedSkPath;
    // Default to use AntiAlias
    bool mAntiAlias = true;
};

class ClipPath : public Path {
public:
    ClipPath(const ClipPath& path) : Path(path) {}
    ClipPath(const char* path, size_t strLength) : Path(path, strLength) {}
    ClipPath() : Path() {}
    void draw(Canvas& outCanvas, bool useStagingData) override;
    void setAntiAlias(bool aa)override {}
};

class Group : public Node {
public:
    class GroupProperties : public Properties {
    public:
        explicit GroupProperties(Node* mNode) : Properties(mNode) {}
        struct PrimitiveFields {
            float rotate = 0;
            float pivotX = 0;
            float pivotY = 0;
            float scaleX = 1;
            float scaleY = 1;
            float translateX = 0;
            float translateY = 0;
        } mPrimitiveFields;
        void syncProperties(const GroupProperties& prop) {
            mPrimitiveFields = prop.mPrimitiveFields;
            onPropertyChanged();
        }
        float getRotation() const { return mPrimitiveFields.rotate; }
        void setRotation(float rotation) { VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(rotate, rotation); }
        float getPivotX() const { return mPrimitiveFields.pivotX; }
        void setPivotX(float pivotX) { VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(pivotX, pivotX); }
        float getPivotY() const { return mPrimitiveFields.pivotY; }
        void setPivotY(float pivotY) { VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(pivotY, pivotY); }
        float getScaleX() const { return mPrimitiveFields.scaleX; }
        void setScaleX(float scaleX) { VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(scaleX, scaleX); }
        float getScaleY() const { return mPrimitiveFields.scaleY; }
        void setScaleY(float scaleY) { VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(scaleY, scaleY); }
        float getTranslateX() const { return mPrimitiveFields.translateX; }
        void setTranslateX(float translateX) {
            VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(translateX, translateX);
        }
        float getTranslateY() const { return mPrimitiveFields.translateY; }
        void setTranslateY(float translateY) {
            VD_SET_PRIMITIVE_FIELD_AND_NOTIFY(translateY, translateY);
        }
        void updateProperties(float rotate, float pivotX, float pivotY, float scaleX, float scaleY,
                              float translateX, float translateY) {
            mPrimitiveFields.rotate = rotate;
            mPrimitiveFields.pivotX = pivotX;
            mPrimitiveFields.pivotY = pivotY;
            mPrimitiveFields.scaleX = scaleX;
            mPrimitiveFields.scaleY = scaleY;
            mPrimitiveFields.translateX = translateX;
            mPrimitiveFields.translateY = translateY;
            onPropertyChanged();
        }
        void setPropertyValue(int propertyId, float value);
        float getPropertyValue(int propertyId) const;
        bool copyProperties(float* outProperties, int length) const;
        static bool isValidProperty(int propertyId);

    private:
        enum class Property {
            rotate = 0,
            pivotX,
            pivotY,
            scaleX,
            scaleY,
            translateX,
            translateY,
            // Count of the properties, must be at the end.
            count,
        };
    };

    Group(const Group& group);
    Group() {}
    void addChild(Node* child);
    virtual void setPropertyChangedListener(PropertyChangedListener* listener) override {
        Node::setPropertyChangedListener(listener);
        for (auto& child : mChildren) {
            child->setPropertyChangedListener(listener);
        }
    }
    virtual void syncProperties() override;
    GroupProperties* mutateStagingProperties() { return &mStagingProperties; }
    const GroupProperties* stagingProperties() { return &mStagingProperties; }

    // This should only be called from animations on RT
    GroupProperties* mutateProperties() { return &mProperties; }

    // Methods below could be called from either UI thread or Render Thread.
    virtual void draw(Canvas& outCanvas, bool useStagingData) override;
    void getLocalMatrix(Cairo::Matrix& outMatrix, const GroupProperties& properties);
    void dump() override;
    static bool isValidProperty(int propertyId);

    void onPropertyChanged(Properties* properties) override {
        if (properties == &mStagingProperties) {
            mStagingPropertiesDirty = true;
            if (mPropertyChangedListener) {
                mPropertyChangedListener->onStagingPropertyChanged();
            }
        } else {
            if (mPropertyChangedListener) {
                mPropertyChangedListener->onPropertyChanged();
            }
        }
    }

    void setAntiAlias(bool aa) override{
        for (auto& child : mChildren) {
            child->setAntiAlias(aa);
        }
    }

    void forEachFillColor(const std::function<void(uint32_t)>& func) const override {
        for (auto& child : mChildren) {
            child->forEachFillColor(func);
        }
    }

private:
    GroupProperties mProperties = GroupProperties(this);
    GroupProperties mStagingProperties = GroupProperties(this);
    bool mStagingPropertiesDirty = true;
    std::vector<Node*> mChildren;
};

using Bitmap=Cairo::RefPtr<Cairo::ImageSurface>;
class Tree{// : public VirtualLightRefBase {
public:
    explicit Tree(Group* rootNode) : mRootNode(rootNode) {
        mRootNode->setPropertyChangedListener(&mPropertyChangedListener);
    }

    // Copy properties from the tree and use the give node as the root node
    Tree(const Tree* copy, Group* rootNode) : Tree(rootNode) {
        mStagingProperties.syncAnimatableProperties(copy->stagingProperties());
        mStagingProperties.syncNonAnimatableProperties(copy->stagingProperties());
    }
    // Draws the VD onto a bitmap cache, then the bitmap cache will be rendered onto the input
    // canvas. Returns the number of pixels needed for the bitmap cache.
    int draw(Canvas& outCanvas, ColorFilter* colorFilter, const Rect& bounds,
             bool needsMirroring, bool canReuseCache);
    void drawStaging(Canvas& canvas);

    Bitmap& getBitmapUpdateIfDirty();
    void setAllowCaching(bool allowCaching) { mAllowCaching = allowCaching; }
    void syncProperties() {
        if (mStagingProperties.mNonAnimatablePropertiesDirty) {
            mCache.dirty |= (mProperties.mNonAnimatableProperties.viewportWidth !=
                             mStagingProperties.mNonAnimatableProperties.viewportWidth) ||
                            (mProperties.mNonAnimatableProperties.viewportHeight !=
                             mStagingProperties.mNonAnimatableProperties.viewportHeight) ||
                            (mProperties.mNonAnimatableProperties.scaledWidth !=
                             mStagingProperties.mNonAnimatableProperties.scaledWidth) ||
                            (mProperties.mNonAnimatableProperties.scaledHeight !=
                             mStagingProperties.mNonAnimatableProperties.scaledHeight) ||
                            (mProperties.mNonAnimatableProperties.bounds !=
                             mStagingProperties.mNonAnimatableProperties.bounds);
            mProperties.syncNonAnimatableProperties(mStagingProperties);
            mStagingProperties.mNonAnimatablePropertiesDirty = false;
        }

        if (mStagingProperties.mAnimatablePropertiesDirty) {
            mProperties.syncAnimatableProperties(mStagingProperties);
        } else {
            mStagingProperties.syncAnimatableProperties(mProperties);
        }
        mStagingProperties.mAnimatablePropertiesDirty = false;
        mRootNode->syncProperties();
    }

    class TreeProperties {
    public:
        explicit TreeProperties(Tree* tree) : mTree(tree) {}
        // Properties that can only be modified by UI thread, therefore sync should
        // only go from UI to RT
        struct NonAnimatableProperties {
            float viewportWidth = 0;
            float viewportHeight = 0;
            Rect bounds{};
            int scaledWidth = 0;
            int scaledHeight = 0;
            ColorFilter* colorFilter=nullptr;
        } mNonAnimatableProperties;
        bool mNonAnimatablePropertiesDirty = true;

        float mRootAlpha = 1.0f;
        bool mAnimatablePropertiesDirty = true;

        void syncNonAnimatableProperties(const TreeProperties& prop) {
            // Copy over the data that can only be changed in UI thread
            if (mNonAnimatableProperties.colorFilter != prop.mNonAnimatableProperties.colorFilter) {
                mNonAnimatableProperties.colorFilter = prop.mNonAnimatableProperties.colorFilter;
            }
            mNonAnimatableProperties = prop.mNonAnimatableProperties;
        }

        void setViewportSize(float width, float height) {
            if (mNonAnimatableProperties.viewportWidth != width ||
                mNonAnimatableProperties.viewportHeight != height) {
                mNonAnimatablePropertiesDirty = true;
                mNonAnimatableProperties.viewportWidth = width;
                mNonAnimatableProperties.viewportHeight = height;
                mTree->onPropertyChanged(this);
            }
        }
        void setBounds(const Rect& bounds) {
            if (mNonAnimatableProperties.bounds != bounds) {
                mNonAnimatableProperties.bounds = bounds;
                mNonAnimatablePropertiesDirty = true;
                mTree->onPropertyChanged(this);
            }
        }

        void setScaledSize(int width, int height) {
            // If the requested size is bigger than what the bitmap was, then
            // we increase the bitmap size to match. The width and height
            // are bound by MAX_CACHED_BITMAP_SIZE.
            if (mNonAnimatableProperties.scaledWidth < width ||
                mNonAnimatableProperties.scaledHeight < height) {
                mNonAnimatableProperties.scaledWidth =
                        std::max(width, mNonAnimatableProperties.scaledWidth);
                mNonAnimatableProperties.scaledHeight =
                        std::max(height, mNonAnimatableProperties.scaledHeight);
                mNonAnimatablePropertiesDirty = true;
                mTree->onPropertyChanged(this);
            }
        }
        void setColorFilter(ColorFilter* filter) {
            if (mNonAnimatableProperties.colorFilter != filter) {
                mNonAnimatableProperties.colorFilter = filter;
                mNonAnimatablePropertiesDirty = true;
                mTree->onPropertyChanged(this);
            }
        }
        ColorFilter* getColorFilter() const { return mNonAnimatableProperties.colorFilter; }

        float getViewportWidth() const { return mNonAnimatableProperties.viewportWidth; }
        float getViewportHeight() const { return mNonAnimatableProperties.viewportHeight; }
        float getScaledWidth() const { return mNonAnimatableProperties.scaledWidth; }
        float getScaledHeight() const { return mNonAnimatableProperties.scaledHeight; }
        void syncAnimatableProperties(const TreeProperties& prop) { mRootAlpha = prop.mRootAlpha; }
        bool setRootAlpha(float rootAlpha) {
            if (rootAlpha != mRootAlpha) {
                mAnimatablePropertiesDirty = true;
                mRootAlpha = rootAlpha;
                mTree->onPropertyChanged(this);
                return true;
            }
            return false;
        }
        float getRootAlpha() const { return mRootAlpha; }
        const Rect& getBounds() const { return mNonAnimatableProperties.bounds; }
        Tree* mTree;
    };
    void onPropertyChanged(TreeProperties* prop);
    TreeProperties* mutateStagingProperties() { return &mStagingProperties; }
    const TreeProperties& stagingProperties() const { return mStagingProperties; }

    // This should only be called from animations on RT
    TreeProperties* mutateProperties() { return &mProperties; }

    // called from RT only
    const TreeProperties& properties() const { return mProperties; }

    // This should always be called from RT.
    void markDirty() { mCache.dirty = true; }
    bool isDirty() const { return mCache.dirty; }
    bool getPropertyChangeWillBeConsumed() const { return mWillBeConsumed; }
    void setPropertyChangeWillBeConsumed(bool willBeConsumed) { mWillBeConsumed = willBeConsumed; }

    /**
     * Draws VD cache into a canvas. This should always be called from RT and it works with Skia
     * pipelines only.
     */
    void draw(Canvas& canvas, const Rect& bounds/*, const SkPaint& paint*/);

    //void getPaintFor(Paint* outPaint, const TreeProperties &props) const;
    //BitmapPalette computePalette();

    void setAntiAlias(bool aa) { mRootNode->setAntiAlias(aa); }

private:
    class Cache {
    public:
        Bitmap bitmap;  // used by HWUI pipeline and software
        bool dirty = true;
    };
    void updatePaint(TreeProperties*prop,Cairo::RefPtr<Cairo::Pattern>&stroke,Cairo::RefPtr<Cairo::Pattern>&fill);
    bool allocateBitmapIfNeeded(Cache& cache, int width, int height);
    bool canReuseBitmap(Bitmap&, int width, int height)const;
    void updateBitmapCache(Bitmap& outCache, bool useStagingData);

    // Cap the bitmap size, such that it won't hurt the performance too much
    // and it won't crash due to a very large scale.
    // The drawable will look blurry above this size.
    const static int MAX_CACHED_BITMAP_SIZE;

    bool mAllowCaching = true;
    Group* mRootNode;

    TreeProperties mProperties = TreeProperties(this);
    TreeProperties mStagingProperties = TreeProperties(this);

    Cache mStagingCache;
    Cache mCache;

    PropertyChangedListener mPropertyChangedListener = PropertyChangedListener(&mCache.dirty, &mStagingCache.dirty);

    mutable bool mWillBeConsumed = false;
};
}/*endof namespace hwui*/
}/*endof namespace cdroid*/
#endif/*__HWUI_VECTOR_DRAWABLE_H__*/
