#ifndef __CDROID_VECTOR_DRAWABLE_H__
#define __CDROID_VECTOR_DRAWABLE_H__
#include <unordered_map>
#include <core/typedvalue.h>
#include <drawables/drawable.h>
#include <animation/property.h>
namespace cdroid{

class PathData;
class VectorDrawableRoot;
using Theme = std::string;
using VirtualRefBasePtr=cdroid::VectorDrawableRoot*;
class VectorDrawable:public Drawable {
public:
    class VectorDrawableState;
private:
    static constexpr const char*const SHAPE_CLIP_PATH = "clip-path";
    static constexpr const char*const SHAPE_GROUP = "group";
    static constexpr const char*const SHAPE_PATH = "path";
    static constexpr const char*const SHAPE_VECTOR = "vector";

    std::shared_ptr<VectorDrawableState> mVectorState;
    PorterDuffColorFilter* mTintFilter;
    ColorFilter* mColorFilter;
    bool mMutated;
    /** The density of the display on which this drawable will be rendered. */
    int mTargetDensity;

    // Given the virtual display setup, the dpi can be different than the inflation's dpi.
    // Therefore, we need to scale the values we got from the getDimension*().
    int mDpiScaledWidth = 0;
    int mDpiScaledHeight = 0;
    Insets mDpiScaledInsets;// = Insets.NONE;

    /** Whether DPI-scaled width, height, and insets need to be updated. */
    bool mDpiScaledDirty = true;
    Rect mTmpBounds;
private:
    VectorDrawable(std::shared_ptr<VectorDrawableState> state);
    //void updateLocalState(Resources res);
    //void updateStateFromTypedArray(TypedArray a);
    void inflateChildElements(Context*,const AttributeSet& attrs,Theme theme);
    bool needMirroring();
protected:
    bool onStateChange(const std::vector<int>& stateSet)override;
    void* getTargetByName(const std::string& name);
    void computeVectorSize();
    void setAllowCaching(bool allowCaching);
public:
    VectorDrawable();

    Drawable* mutate() override;
    void clearMutated() override;
    std::shared_ptr<ConstantState> getConstantState() override;
    void draw(Canvas& canvas) override;
    int getAlpha()const override;
    void setAlpha(int alpha)override;
    void setColorFilter(ColorFilter* colorFilter)override;
    ColorFilter* getColorFilter()override;
    void setTintList(const ColorStateList* tint)override;
    void setTintMode(int tintMode);
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    int getOpacity() override;
    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    Insets getOpticalInsets() override;
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
    //void inflate(Resources r, XmlPullParser parser,AttributeSet attrs,Theme theme);
    int getChangingConfigurations()const override;
    void setAutoMirrored(bool mirrored) override;
    bool isAutoMirrored() override;
    long getNativeTree();
    void setAntiAlias(bool aa);
public:
    class VectorDrawableState;
    class VGroup;
    class VPath;
    class VClipPath;
    class VFullPath;

    class VObject {
        friend VGroup;
        VirtualRefBasePtr mTreePtr = nullptr;
    public:
        bool isTreeValid() {
            return mTreePtr != nullptr;// && mTreePtr.get() != 0;
        }
        virtual void setTree(VirtualRefBasePtr ptr) {
            mTreePtr = ptr;
        }
        virtual long getNativePtr()=0;
        virtual void inflate(Context*,const AttributeSet& attrs, Theme theme)=0;
        virtual bool canApplyTheme()=0;
        virtual void applyTheme(Theme t)=0;
        virtual bool onStateChange(const std::vector<int>& state)=0;
        virtual bool isStateful()const=0;
        virtual bool hasFocusStateSpecified()const=0;
        virtual int getNativeSize()const =0;
        virtual Property* getProperty(const std::string& propertyName)=0;
    };
private:
    static int nDraw(long rendererPtr, long canvasWrapperPtr,
           long colorFilterPtr, Rect bounds, bool needsMirroring, bool canReuseCache);
    static bool nGetFullPathProperties(long pathPtr, uint8_t* properties,int length);
    static void nSetName(long nodePtr, const std::string& name);
    static bool nGetGroupProperties(long groupPtr, float* properties,int length);
    static void nSetPathString(long pathPtr, const std::string& pathString, int length);

    // ------------- @FastNative ------------------

    static long nCreateTree(long rootGroupPtr);
    static long nCreateTreeFromCopy(long treeToCopy, long rootGroupPtr);
    static void nSetRendererViewportSize(long rendererPtr, float viewportWidth,float viewportHeight);
    static bool nSetRootAlpha(long rendererPtr, float alpha);
    static float nGetRootAlpha(long rendererPtr);
    static void nSetAntiAlias(long rendererPtr, bool aa);
    static void nSetAllowCaching(long rendererPtr, bool allowCaching);

    static long nCreateFullPath();
    static long nCreateFullPath(long nativeFullPathPtr);

    static void nUpdateFullPathProperties(long pathPtr, float strokeWidth,
        int strokeColor, float strokeAlpha, int fillColor, float fillAlpha, float trimPathStart,
        float trimPathEnd, float trimPathOffset, float strokeMiterLimit, int strokeLineCap,
        int strokeLineJoin, int fillType);
    static void nUpdateFullPathFillGradient(long pathPtr, long fillGradientPtr);
    static void nUpdateFullPathStrokeGradient(long pathPtr, long strokeGradientPtr);

    static long nCreateClipPath();
    static long nCreateClipPath(long clipPathPtr);

    static long nCreateGroup();
    static long nCreateGroup(long groupPtr);
    static void nUpdateGroupProperties(long groupPtr, float rotate, float pivotX,
            float pivotY, float scaleX, float scaleY, float translateX, float translateY);

    static void nAddChild(long groupPtr, long nodePtr);

    /**
     * The setters and getters below for paths and groups are here temporarily, and will be
     * removed once the animation in AVD is replaced with RenderNodeAnimator, in which case the
     * animation will modify these properties in native. By then no JNI hopping would be necessary
     * for VD during animation, and these setters and getters will be obsolete.
     */
    // Setters and getters during animation.
    static float nGetRotation(long groupPtr);
    static void nSetRotation(long groupPtr, float rotation);
    static float nGetPivotX(long groupPtr);
    static void nSetPivotX(long groupPtr, float pivotX);
    static float nGetPivotY(long groupPtr);
    static void nSetPivotY(long groupPtr, float pivotY);
    static float nGetScaleX(long groupPtr);
    static void nSetScaleX(long groupPtr, float scaleX);
    static float nGetScaleY(long groupPtr);
    static void nSetScaleY(long groupPtr, float scaleY);
    static float nGetTranslateX(long groupPtr);
    static void nSetTranslateX(long groupPtr, float translateX);
    static float nGetTranslateY(long groupPtr);
    static void nSetTranslateY(long groupPtr, float translateY);

    // Setters and getters for VPath during animation.
    static void nSetPathData(long pathPtr, long pathDataPtr);
    static float nGetStrokeWidth(long pathPtr);
    static void nSetStrokeWidth(long pathPtr, float width);
    static int nGetStrokeColor(long pathPtr);
    static void nSetStrokeColor(long pathPtr, int strokeColor);
    static float nGetStrokeAlpha(long pathPtr);
    static void nSetStrokeAlpha(long pathPtr, float alpha);
    static int nGetFillColor(long pathPtr);
    static void nSetFillColor(long pathPtr, int fillColor);
    static float nGetFillAlpha(long pathPtr);
    static void nSetFillAlpha(long pathPtr, float fillAlpha);
    static float nGetTrimPathStart(long pathPtr);
    static void nSetTrimPathStart(long pathPtr, float trimPathStart);
    static float nGetTrimPathEnd(long pathPtr);
    static void nSetTrimPathEnd(long pathPtr, float trimPathEnd);
    static float nGetTrimPathOffset(long pathPtr);
    static void nSetTrimPathOffset(long pathPtr, float trimPathOffset);
};

class VectorDrawable::VectorDrawableState:public std::enable_shared_from_this<VectorDrawableState> ,public ConstantState {
protected:
    friend VectorDrawable;
    // Variables below need to be copied (deep copy if applicable) for mutation.
    int mThemeAttrs[2];
    int mChangingConfigurations;
    ColorStateList* mTint = nullptr;
    int mTintMode = DEFAULT_TINT_MODE;
    bool mAutoMirrored;

    int mBaseWidth = 0;
    int mBaseHeight = 0;
    float mViewportWidth = 0;
    float mViewportHeight = 0;
    Insets mOpticalInsets;// = Insets.NONE;
    std::string mRootName;
    VGroup* mRootGroup;
    VirtualRefBasePtr mNativeTree = nullptr;

    int mDensity = DisplayMetrics::DENSITY_DEFAULT;
    std::unordered_map<std::string,void*> mVGTargetsMap;

    // Fields for cache
    int mCachedThemeAttrs[2];
    ColorStateList* mCachedTint;
    PorterDuff::Mode mCachedTintMode;
    bool mCachedAutoMirrored;
    bool mCacheDirty;

    // Since sw canvas and hw canvas uses different bitmap caches, we track the allocation of
    // these bitmaps separately.
    int mLastSWCachePixelCount = 0;
    int mLastHWCachePixelCount = 0;

    static Property* /*<VectorDrawableState, float>*/ ALPHA;
    // This tracks the total native allocation for all the nodes.
    int mAllocationOfAllNodes = 0;

    static constexpr int NATIVE_ALLOCATION_SIZE = 316;
private:
    void createNativeTree(VGroup* rootGroup);
    void createNativeTreeFromCopy(const VectorDrawableState& copy, VGroup* rootGroup);
    void applyDensityScaling(int sourceDensity, int targetDensity);
protected:
    Property* getProperty(const std::string& propertyName);
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

    static constexpr int NATIVE_ALLOCATION_SIZE = 100;

    static std::unordered_map<std::string, int> sPropertyIndexMap;

    static int getPropertyIndex(const std::string& propertyName);

    // Below are the Properties that wrap the setters to avoid reflection overhead in animations
    static Property* /*<VGroup, float>*/ TRANSLATE_X;
    static Property* /*<VGroup, float>*/ TRANSLATE_Y;
    static Property* /*<VGroup, float>*/ SCALE_X;
    static Property* /*<VGroup, float>*/ SCALE_Y;
    static Property* /*<VGroup, float>*/ PIVOT_X;
    static Property* /*<VGroup, float>*/ PIVOT_Y;
    static Property* /*<VGroup, float>*/ ROTATION;
    static std::unordered_map<std::string, Property*> sPropertyMap;
    // Temp array to store transform values obtained from native.
    float mTransform[6];
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
    long mNativePtr;
public:
    VGroup(const VGroup* copy,std::unordered_map<std::string, void*>& targetsMap);
    VGroup();

    Property* getProperty(const std::string& propertyName)override;

    std::string getGroupName()const;

    void addChild(VObject* child);
    void setTree(VirtualRefBasePtr treeRoot)override;
    long getNativePtr();
    void inflate(Context*,const AttributeSet& attrs, Theme theme);
    //void updateStateFromTypedArray(TypedArray a);
    bool onStateChange(const std::vector<int>& stateSet);
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    int getNativeSize()const override;

    bool canApplyTheme()override;
    void applyTheme(Theme t);
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
};

/**
 * Common Path information for clip path and normal path.
 */
class VectorDrawable::VPath:public VectorDrawable::VObject {
protected:
    friend VGroup;
    PathData* mPathData = nullptr;
    std::string mPathName;
    int mChangingConfigurations;
    static Property* /*<VPath*, PathData*>*/ PATH_DATA;
public:
    VPath();
    VPath(const VPath* copy);
    Property* getProperty(const std::string& propertyName);
    std::string getPathName()const;
    /* Setters and Getters, used by animator from AnimatedVectorDrawable. */
    PathData* getPathData();
    // TODO: Move the PathEvaluator and this setter and the getter above into native.
    void setPathData(PathData* pathData);
};

/**
 * Clip path, which only has name and pathData.
 */
class VectorDrawable::VClipPath:public VPath {
private:
    long mNativePtr;
    static constexpr int NATIVE_ALLOCATION_SIZE = 120;
public:
    VClipPath();

    VClipPath(const VClipPath* copy);
    long getNativePtr();
    void inflate(Context*,const AttributeSet& attrs, Theme theme);
    bool canApplyTheme() override;

    void applyTheme(Theme theme) override;
    bool onStateChange(const std::vector<int>& stateSet) override;
    bool isStateful() const override;
    bool hasFocusStateSpecified() const override;
    int getNativeSize() const override;
    //void updateStateFromTypedArray(TypedArray a);
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

    static constexpr int NATIVE_ALLOCATION_SIZE = 264;
    // Property map for animatable attributes.
    static std::unordered_map<std::string, int> sPropertyIndexMap;
    // Below are the Properties that wrap the setters to avoid reflection overhead in animations
    static Property* /*<VFullPath, float>*/ STROKE_WIDTH;
    static Property* /*<VFullPath, int>*/ STROKE_COLOR;
    static Property* /*<VFullPath, float>*/ STROKE_ALPHA;
    static Property* /*<VFullPath, int>*/ FILL_COLOR;
    static Property* /*<VFullPath, float>*/ FILL_ALPHA;
    static Property* /*<VFullPath, float>*/ TRIM_PATH_START;
    static Property* /*<VFullPath, float>*/ TRIM_PATH_END;
    static Property* /*<VFullPath, float>*/TRIM_PATH_OFFSET;
    static std::unordered_map<std::string, Property*> sPropertyMap;

    // Temp array to store property data obtained from native getter.
    uint8_t* mPropertyData;
    /////////////////////////////////////////////////////
    // Variables below need to be copied (deep copy if applicable) for mutation.
    int mThemeAttrs[2];

    ComplexColor* mStrokeColors = nullptr;
    ComplexColor* mFillColors = nullptr;
    long mNativePtr;
private:
    //void updateStateFromTypedArray(TypedArray a);
    bool canComplexColorApplyTheme(ComplexColor* complexColor);
public:
    VFullPath();
    VFullPath(const VFullPath* copy);
    Property* getProperty(const std::string& propertyName)override;
    int getPropertyIndex(const std::string& propertyName);
    bool onStateChange(const std::vector<int>& stateSet) override;
    bool isStateful() const override;
    bool hasFocusStateSpecified()const override;
    int getNativeSize() const override;
    long getNativePtr() override;
    void inflate(Context*,const AttributeSet& attrs, Theme theme);

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
