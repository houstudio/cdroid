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
#ifndef __CDROID_VECTOR_DRAWABLE_H__
#define __CDROID_VECTOR_DRAWABLE_H__
#include <unordered_map>
#include <core/typedvalue.h>
#include <drawable/drawable.h>
#include <drawable/pathparser.h>
#include <animation/property.h>
namespace cdroid{
namespace hwui{
    class Group;
    class Path;
    class ClipPath;
    class FullPath;
    class Tree;
}
using Theme = std::string;
class AnimatedVectorDrawable;
class VectorDrawable:public Drawable {
public:
    class VectorDrawableState;
    class VGroup;
    class VPath;
    class VClipPath;
    class VFullPath;
    friend AnimatedVectorDrawable;
private:
    static constexpr const char*const SHAPE_CLIP_PATH = "clip-path";
    static constexpr const char*const SHAPE_GROUP = "group";
    static constexpr const char*const SHAPE_PATH = "path";
    static constexpr const char*const SHAPE_VECTOR = "vector";

    std::shared_ptr<VectorDrawableState> mVectorState;
    PorterDuffColorFilter* mTintFilter;
    ColorFilter* mColorFilter;
    bool mMutated;
    /** Whether DPI-scaled width, height, and insets need to be updated. */
    bool mDpiScaledDirty = true;
    /** The density of the display on which this drawable will be rendered. */
    int mTargetDensity;
    // Given the virtual display setup, the dpi can be different than the inflation's dpi.
    // Therefore, we need to scale the values we got from the getDimension*().
    int mDpiScaledWidth = 0;
    int mDpiScaledHeight = 0;
    Insets mDpiScaledInsets;// = Insets.NONE;
private:
    VectorDrawable(std::shared_ptr<VectorDrawableState> state);
    void updateLocalState();
    bool needMirroring();
    void updateStateFromTypedArray(const AttributeSet&atts);
    void inflateChildElements(XmlPullParser&parser,const AttributeSet&);
protected:
    bool onStateChange(const std::vector<int>& stateSet)override;
    void computeVectorSize();
    void setAllowCaching(bool allowCaching);
public:
    VectorDrawable();
    ~VectorDrawable()override;
    Drawable* mutate() override;
    void clearMutated() override;
    std::shared_ptr<ConstantState> getConstantState() override;
    void draw(Canvas& canvas) override;
    int getAlpha()const override;
    void setAlpha(int alpha)override;
    void setColorFilter(ColorFilter* colorFilter)override;
    ColorFilter* getColorFilter()override;
    void setTintList(const ColorStateList* tint)override;
    void setTintMode(int tintMode)override;
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    int getOpacity() override;
    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    Insets getOpticalInsets() override;
    void* getTargetByName(const std::string& name);
    /*
     * Update local dimensions to adjust for a target density that may differ
     * from the source density against which the constant state was loaded.
     */
    bool canApplyTheme() override;
    //void applyTheme(Theme t) override;

    /**
     * The size of a pixel when scaled from the intrinsic dimension to the viewport dimension.
     * This is used to calculate the path animation accuracy.
     *
     * @hide
     */
    float getPixelSize();
    static VectorDrawable* create(Context*,const std::string&resId);
    int getChangingConfigurations()const override;
    void setAutoMirrored(bool mirrored) override;
    bool isAutoMirrored() const override;
    long getNativeTree();
    void setAntiAlias(bool aa);
    void inflate(XmlPullParser&,const AttributeSet&)override;
public:
    class VObject {
        friend VectorDrawableState;
        friend VGroup;
        hwui::Tree* mTreePtr = nullptr;
    public:
        virtual ~VObject() = default;
        bool isTreeValid() {
            return mTreePtr != nullptr;// && mTreePtr.get() != 0;
        }
        virtual void setTree(hwui::Tree* ptr) {
            mTreePtr = ptr;
        }
        virtual long getNativePtr()=0;
        virtual void inflate(XmlPullParser&,const AttributeSet& attrs)=0;
        virtual bool canApplyTheme()=0;
        virtual void applyTheme(Theme t)=0;
        virtual bool onStateChange(const std::vector<int>& state)=0;
        virtual bool isStateful()const=0;
        virtual bool hasFocusStateSpecified()const=0;
        virtual const Property* getProperty(const std::string& propertyName)=0;
    };
};

class VectorDrawable::VectorDrawableState:public std::enable_shared_from_this<VectorDrawableState> ,public ConstantState {
protected:
    friend VectorDrawable;
    friend AnimatedVectorDrawable;
    // Variables below need to be copied (deep copy if applicable) for mutation.
    int mThemeAttrs[2];
    int mChangingConfigurations;
    ColorStateList* mTint = nullptr;
    int mTintMode = DEFAULT_TINT_MODE;
    int mBaseWidth = 0;
    int mBaseHeight = 0;
    float mViewportWidth = 0;
    float mViewportHeight = 0;
    Insets mOpticalInsets;// = Insets.NONE;
    std::string mRootName;
    VGroup* mRootGroup;
    hwui::Tree* mNativeTree = nullptr;

    int mDensity = DisplayMetrics::DENSITY_DEFAULT;
    std::unordered_map<std::string,void*> mVGTargetsMap;

    // Fields for cache
    int mCachedThemeAttrs[2];
    ColorStateList* mCachedTint;
    PorterDuff::Mode mCachedTintMode;
    bool mAutoMirrored;
    bool mCachedAutoMirrored;
    bool mCacheDirty;

    // Since sw canvas and hw canvas uses different bitmap caches, we track the allocation of
    // these bitmaps separately.
    int mLastSWCachePixelCount = 0;
    int mLastHWCachePixelCount = 0;

    static const FloatProperty& ALPHA;
    // This tracks the total native allocation for all the nodes.
private:
    void createNativeTree(VGroup* rootGroup);
    void createNativeTreeFromCopy(const VectorDrawableState* copy, VGroup* rootGroup);
    void applyDensityScaling(int sourceDensity, int targetDensity);
protected:
    const Property* getProperty(const std::string& propertyName);
public:
    // If copy is not null, deep copy the given VectorDrawableState. Otherwise, create a
    // native vector drawable tree with an empty root group.
    VectorDrawableState(const VectorDrawableState* copy);
    ~VectorDrawableState();
    // This should be called every time after a new RootGroup and all its subtrees are created
    // (i.e. in constructors of VectorDrawableState and in inflate).
    void onTreeConstructionFinished();
    long getNativeRenderer();
    bool canReuseCache();
    void updateCacheStates();
    void applyTheme(Theme t);
    bool canApplyTheme();
    Drawable* newDrawable()override;
    int getChangingConfigurations()const override;
    bool isStateful()const;
    bool hasFocusStateSpecified()const;
    void setViewportSize(float viewportWidth, float viewportHeight);
    bool setDensity(int targetDensity);
    bool onStateChange(const std::vector<int>& stateSet);
    bool setAlpha(float alpha);
    float getAlpha();
};

class VectorDrawable::VGroup:public VectorDrawable::VObject {
private:
    static constexpr int ROTATION_INDEX = 0;
    static constexpr int PIVOT_X_INDEX = 1;
    static constexpr int PIVOT_Y_INDEX = 2;
    static constexpr int SCALE_X_INDEX = 3;
    static constexpr int SCALE_Y_INDEX = 4;
    static constexpr int TRANSLATE_X_INDEX = 5;
    static constexpr int TRANSLATE_Y_INDEX = 6;
    static constexpr int TRANSFORM_PROPERTY_COUNT = 7;

    static std::unordered_map<std::string, int> sPropertyIndexMap;
    static int getPropertyIndex(const std::string& propertyName);

    static const std::unordered_map<std::string,const Property*> sPropertyMap;
    // Temp array to store transform values obtained from native.
    //float mTransform[8];
    /////////////////////////////////////////////////////
    // Variables below need to be copied (deep copy if applicable) for mutation.
    std::vector<VObject*> mChildren;
    bool mIsStateful;

    // mLocalMatrix is updated based on the update of transformation information,
    // either parsed from the XML or by animation.
    int mChangingConfigurations;
    int mThemeAttrs[2];
    std::string mGroupName;

    // The native object will be created in the constructor and will be destroyed in native
    // when the neither java nor native has ref to the tree. This pointer should be valid
    // throughout this VGroup Java object's life.
    hwui::Group* mNativePtr;
    friend VectorDrawable;
    friend VectorDrawableState;
    friend AnimatedVectorDrawable;

    static const FloatProperty*const TRANSLATE_X;
    static const FloatProperty*const TRANSLATE_Y;
    static const FloatProperty*const SCALE_X;
    static const FloatProperty*const SCALE_Y;
    static const FloatProperty*const PIVOT_X;
    static const FloatProperty*const PIVOT_Y;
    static const FloatProperty*const ROTATION;
public:
    VGroup();
    ~VGroup();
    VGroup(const VGroup* copy,std::unordered_map<std::string, void*>& targetsMap);
    const Property* getProperty(const std::string& propertyName)override;

    std::string getGroupName()const;

    void addChild(VObject* child);
    void setTree(hwui::Tree* treeRoot)override;
    long getNativePtr()override;
    void updateStateFromTypedArray(Context*,const AttributeSet&atts);
    bool onStateChange(const std::vector<int>& stateSet)override;
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;

    bool canApplyTheme()override;
    void applyTheme(Theme t)override;
    /* Setters and Getters, used by animator from AnimatedVectorDrawable. */
    float getRotation();
    void setRotation(float rotation);
    float getPivotX();
    void setPivotX(float pivotX);
    float getPivotY();
    void setPivotY(float pivotY);
    float getScaleX();
    void setScaleX(float scaleX);
    float getScaleY();
    void setScaleY(float scaleY);
    float getTranslateX();
    void setTranslateX(float translateX);
    float getTranslateY();
    void setTranslateY(float translateY);
    void inflate(XmlPullParser&,const AttributeSet&atts)override;
};

/**
 * Common Path information for clip path and normal path.
 */
class VectorDrawable::VPath:public VectorDrawable::VObject {
protected:
    friend VGroup;
    friend VectorDrawable;
    friend AnimatedVectorDrawable;
    PathParser::PathData* mPathData = nullptr;
    std::string mPathName;
    int mChangingConfigurations;
    static const Property*const PATH_DATA;
public:
    VPath();
    VPath(const VPath* copy);
    ~VPath()override;
    const Property* getProperty(const std::string& propertyName)override;
    std::string getPathName()const;
    /* Setters and Getters, used by animator from AnimatedVectorDrawable. */
    PathParser::PathData* getPathData();
    // TODO: Move the PathEvaluator and this setter and the getter above into native.
    void setPathData(const PathParser::PathData* pathData);
};

/**
 * Clip path, which only has name and pathData.
 */
class VectorDrawable::VClipPath:public VPath {
private:
    hwui::ClipPath* mNativePtr;
public:
    VClipPath();
    VClipPath(const VClipPath* copy);
    ~VClipPath()override;
    long getNativePtr()override;
    bool canApplyTheme() override;
    void applyTheme(Theme theme) override;
    bool onStateChange(const std::vector<int>& stateSet) override;
    bool isStateful() const override;
    bool hasFocusStateSpecified() const override;
    void updateStateFromTypedArray(const AttributeSet&atts);
    void inflate(XmlPullParser&,const AttributeSet& attrs)override;
};

/**
 * Normal path, which contains all the fill / paint information.
 */
class VectorDrawable::VFullPath:public VPath {
private:
    static constexpr int STROKE_WIDTH_INDEX = 0;
    static constexpr int STROKE_COLOR_INDEX = 1;
    static constexpr int STROKE_ALPHA_INDEX = 2;
    static constexpr int FILL_COLOR_INDEX = 3;
    static constexpr int FILL_ALPHA_INDEX = 4;
    static constexpr int TRIM_PATH_START_INDEX = 5;
    static constexpr int TRIM_PATH_END_INDEX = 6;
    static constexpr int TRIM_PATH_OFFSET_INDEX = 7;
    static constexpr int STROKE_LINE_CAP_INDEX = 8;
    static constexpr int STROKE_LINE_JOIN_INDEX = 9;
    static constexpr int STROKE_MITER_LIMIT_INDEX = 10;
    static constexpr int FILL_TYPE_INDEX = 11;
    static constexpr int TOTAL_PROPERTY_COUNT = 12;

    // Property map for animatable attributes.
    static std::unordered_map<std::string, int> sPropertyIndexMap;
    static const std::unordered_map<std::string, const Property*> sPropertyMap;

    // Temp array to store property data obtained from native getter.
    uint8_t* mPropertyData;
    /////////////////////////////////////////////////////
    // Variables below need to be copied (deep copy if applicable) for mutation.
    int mThemeAttrs[2];

    ComplexColor* mStrokeColors = nullptr;
    ComplexColor* mFillColors = nullptr;
    hwui::FullPath* mNativePtr;
    static const FloatProperty*const STROKE_WIDTH;
    static const Property*const STROKE_COLOR;
    static const FloatProperty*const STROKE_ALPHA;
    static const Property*const FILL_COLOR;
    static const FloatProperty*const FILL_ALPHA;
    static const FloatProperty*const TRIM_PATH_START;
    static const FloatProperty*const TRIM_PATH_END;
    static const FloatProperty*const TRIM_PATH_OFFSET;
private:
    void updateStateFromTypedArray(const AttributeSet&atts);
    bool canComplexColorApplyTheme(ComplexColor* complexColor);
    void inflateGradients(XmlPullParser&,const AttributeSet&atts);
public:
    VFullPath();
    VFullPath(const VFullPath* copy);
    ~VFullPath()override;
    const Property* getProperty(const std::string& propertyName)override;
    int getPropertyIndex(const std::string& propertyName);
    bool onStateChange(const std::vector<int>& stateSet) override;
    bool isStateful() const override;
    bool hasFocusStateSpecified()const override;
    long getNativePtr() override;
    void inflate(XmlPullParser&,const AttributeSet& attrs)override;

    bool canApplyTheme()override;
    void applyTheme(Theme t)override;

    /* Setters and Getters, used by animator from AnimatedVectorDrawable. */
    int getStrokeColor();
    void setStrokeColor(int strokeColor);
    float getStrokeWidth();
    void setStrokeWidth(float strokeWidth);
    float getStrokeAlpha();
    void setStrokeAlpha(float strokeAlpha);
    int getFillColor();
    void setFillColor(int fillColor);
    float getFillAlpha();
    void setFillAlpha(float fillAlpha);
    float getTrimPathStart();
    void setTrimPathStart(float trimPathStart);
    float getTrimPathEnd();
    void setTrimPathEnd(float trimPathEnd);
    float getTrimPathOffset();
    void setTrimPathOffset(float trimPathOffset);
};
}/*endof namespace*/
#endif/*__VECTOR_DRAWABLE_H__*/
