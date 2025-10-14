#ifndef __POINTER_ICON_H__
#define __POINTER_ICON_H__
#include <cairomm/surface.h>
#include <core/context.h>
#include <core/sparsearray.h>
#include <drawable/bitmapdrawable.h>

namespace cdroid {
using Bitmap = Cairo::RefPtr<Cairo::ImageSurface>;
class PointerIcon{
    /** {@hide} Type constant: Custom icon with a user-supplied bitmap. */
public:
    static constexpr int TYPE_CUSTOM = -1;

    /** Type constant: Null icon.  It has no bitmap. */
    static constexpr int TYPE_NULL = 0;

    /** Type constant: no icons are specified. If all views uses this, then falls back
     * to the default type, but this is helpful to distinguish a view explicitly want
     * to have the default icon.
     * @hide
     */
    static constexpr int TYPE_NOT_SPECIFIED = 1;

    /** Type constant: Arrow icon.  (Default mouse pointer) */
    static constexpr int TYPE_ARROW = 1000;

    /** {@hide} Type constant: Spot hover icon for touchpads. */
    static constexpr int TYPE_SPOT_HOVER = 2000;

    /** {@hide} Type constant: Spot touch icon for touchpads. */
    static constexpr int TYPE_SPOT_TOUCH = 2001;

    /** {@hide} Type constant: Spot anchor icon for touchpads. */
    static constexpr int TYPE_SPOT_ANCHOR = 2002;

    // Type constants for additional predefined icons for mice.
    /** Type constant: context-menu. */
    static constexpr int TYPE_CONTEXT_MENU = 1001;

    /** Type constant: hand. */
    static constexpr int TYPE_HAND = 1002;

    /** Type constant: help. */
    static constexpr int TYPE_HELP = 1003;

    /** Type constant: wait. */
    static constexpr int TYPE_WAIT = 1004;

    /** Type constant: cell. */
    static constexpr int TYPE_CELL = 1006;

    /** Type constant: crosshair. */
    static constexpr int TYPE_CROSSHAIR = 1007;

    /** Type constant: text. */
    static constexpr int TYPE_TEXT = 1008;

    /** Type constant: vertical-text. */
    static constexpr int TYPE_VERTICAL_TEXT = 1009;

    /** Type constant: alias (indicating an alias of/shortcut to something is
      * to be created. */
    static constexpr int TYPE_ALIAS = 1010;

    /** Type constant: copy. */
    static constexpr int TYPE_COPY = 1011;

    /** Type constant: no-drop. */
    static constexpr int TYPE_NO_DROP = 1012;

    /** Type constant: all-scroll. */
    static constexpr int TYPE_ALL_SCROLL = 1013;

    /** Type constant: horizontal double arrow mainly for resizing. */
    static constexpr int TYPE_HORIZONTAL_DOUBLE_ARROW = 1014;

    /** Type constant: vertical double arrow mainly for resizing. */
    static constexpr int TYPE_VERTICAL_DOUBLE_ARROW = 1015;

    /** Type constant: diagonal double arrow -- top-right to bottom-left. */
    static constexpr int TYPE_TOP_RIGHT_DIAGONAL_DOUBLE_ARROW = 1016;

    /** Type constant: diagonal double arrow -- top-left to bottom-right. */
    static constexpr int TYPE_TOP_LEFT_DIAGONAL_DOUBLE_ARROW = 1017;

    /** Type constant: zoom-in. */
    static constexpr int TYPE_ZOOM_IN = 1018;

    /** Type constant: zoom-out. */
    static constexpr int TYPE_ZOOM_OUT = 1019;

    /** Type constant: grab. */
    static constexpr int TYPE_GRAB = 1020;

    /** Type constant: grabbing. */
    static constexpr int TYPE_GRABBING = 1021;

    /** The default pointer icon. */
    static constexpr int TYPE_DEFAULT = TYPE_ARROW;
    // OEM private types should be defined starting at this range to avoid
    // conflicts with any system types that may be defined in the future.
private:
    static constexpr int TYPE_OEM_FIRST = 10000;

    static PointerIcon* gNullIcon;// = new PointerIcon(TYPE_NULL);
    static SparseArray<PointerIcon*> gSystemIcons;
    static bool sUseLargeIcons;

    int mType;
    std::string mSystemIconResourceId;
    Bitmap mBitmap;
    float mHotSpotX;
    float mHotSpotY;
    // The bitmaps for the additional frame of animated pointer icon. Note that the first frame
    // will be stored in mBitmap.
    std::vector<Bitmap> mBitmapFrames;
    int mDurationPerFrame;

private:
    PointerIcon(int type);
public:
    /**
     * Gets a special pointer icon that has no bitmap.
     *
     * @return The null pointer icon.
     *
     * @see #TYPE_NULL
     * @hide
     */
    static PointerIcon* getNullIcon();

    /**
     * Gets the default pointer icon.
     *
     * @param context The context.
     * @return The default pointer icon.
     *
     * @throws IllegalArgumentException if context is null.
     * @hide
     */
    static PointerIcon* getDefaultIcon(Context* context);

    /**
     * Gets a system pointer icon for the given type.
     * If typeis not recognized, returns the default pointer icon.
     *
     * @param context The context.
     * @param type The pointer icon type.
     * @return The pointer icon.
     *
     * @throws IllegalArgumentException if context is null.
     */
    static PointerIcon* getSystemIcon(Context* context, int type);

    /**
     * Updates wheter accessibility large icons are used or not.
     * @hide
     */
    static void setUseLargeIcons(bool use);

    /**
     * Creates a custom pointer icon from the given bitmap and hotspot information.
     *
     * @param bitmap The bitmap for the icon.
     * @param hotSpotX The X offset of the pointer icon hotspot in the bitmap.
     *        Must be within the [0, bitmap.getWidth()) range.
     * @param hotSpotY The Y offset of the pointer icon hotspot in the bitmap.
     *        Must be within the [0, bitmap.getHeight()) range.
     * @return A pointer icon for this bitmap.
     *
     * @throws IllegalArgumentException if bitmap is null, or if the x/y hotspot
     *         parameters are invalid.
     */
    static PointerIcon* create(Bitmap bitmap, float hotSpotX, float hotSpotY);

    /**
     * Loads a custom pointer icon from an XML resource.
     * <p>
     * The XML resource should have the following form:
     * <code>
     * &lt;?xml version="1.0" encoding="utf-8"?&gt;
     * &lt;pointer-icon xmlns:android="http://schemas.android.com/apk/res/android"
     *   android:bitmap="@drawable/my_pointer_bitmap"
     *   android:hotSpotX="24"
     *   android:hotSpotY="24" /&gt;
     * </code>
     * </p>
     *
     * @param resources The resources object.
     * @param resourceId The resource id.
     * @return The pointer icon.
     *
     * @throws IllegalArgumentException if resources is null.
     * @throws Resources.NotFoundException if the resource was not found or the drawable
     * linked in the resource was not found.
     */
    static PointerIcon* load(Context*ctx,const std::string& resourceId);

    /**
     * Loads the bitmap and hotspot information for a pointer icon, if it is not already loaded.
     * Returns a pointer icon (not necessarily the same instance) with the information filled in.
     *
     * @param context The context.
     * @return The loaded pointer icon.
     *
     * @throws IllegalArgumentException if context is null.
     * @hide
     */
    PointerIcon* load(Context* context);

    /** @hide */
    int getType() const;

    bool equals(const PointerIcon* other);
    /**
     *  Get the Bitmap from the Drawable.
     *
     *  If the Bitmap needed to be scaled up to account for density, BitmapDrawable
     *  handles this at draw time. But this class doesn't actually draw the Bitmap;
     *  it is just a holder for native code to access its SkBitmap. So this needs to
     *  get a version that is scaled to account for density.
     */
private:
    Bitmap getBitmapFromDrawable(BitmapDrawable* bitmapDrawable);

    void loadResource(Context* context, const std::string& resourceId);

    static void validateHotSpot(Bitmap bitmap, float hotSpotX, float hotSpotY);

    static std::string getSystemIconTypeIndex(int type);
};
}/*endof namespace*/
#endif/*__POINTER_ICON_H__*/

