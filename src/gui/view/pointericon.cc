#include <view/pointericon.h>
#include <porting/cdlog.h>
#include <drawable/animationdrawable.h>
#include <core/xmlpullparser.h>
#include <stdexcept>

namespace cdroid{

PointerIcon* PointerIcon::gNullIcon = new PointerIcon(TYPE_NULL);
SparseArray<PointerIcon*> PointerIcon::gSystemIcons;
bool PointerIcon::sUseLargeIcons = false;

PointerIcon::PointerIcon(int type) {
    mType = type;
}

PointerIcon* PointerIcon::getNullIcon() {
    return gNullIcon;
}

PointerIcon* PointerIcon::getDefaultIcon(Context* context) {
    return getSystemIcon(context, TYPE_DEFAULT);
}

PointerIcon* PointerIcon::getSystemIcon(Context* context, int type) {
    if (context == nullptr) {
        throw std::runtime_error("context must not be null");
    }

    if (type == TYPE_NULL) {
        return gNullIcon;
    }

    PointerIcon* icon = gSystemIcons.get(type);
    if (icon != nullptr) {
        return icon;
    }

    std::string typeIndex = getSystemIconTypeIndex(type);
    if (typeIndex.empty()) {
        typeIndex = getSystemIconTypeIndex(TYPE_DEFAULT);
    }

    //const int defStyle = sUseLargeIcons ? com.android.internal.R.style.LargePointer : com.android.internal.R.style.Pointer;
    //TypedArray a = context.obtainStyledAttributes(nullptr, com.android.internal.R.styleable.Pointer,0, defStyle);
    const std::string resourceId;// = a.getResourceId(typeIndex, -1);

    if (resourceId.empty()) {
        LOGW("Missing theme resources for pointer icon type %d",type);
        return type == TYPE_DEFAULT ? gNullIcon : getSystemIcon(context, TYPE_DEFAULT);
    }

    icon = new PointerIcon(type);
    /*if ((resourceId & 0xff000000) == 0x01000000) {
        icon->mSystemIconResourceId = resourceId;
    } else */{
        icon->loadResource(context, resourceId);
    }
    gSystemIcons.append(type, icon);
    return icon;
}

void PointerIcon::setUseLargeIcons(bool use) {
    sUseLargeIcons = use;
    gSystemIcons.clear();
}

PointerIcon* PointerIcon::create(Bitmap bitmap, float hotSpotX, float hotSpotY) {
    if (bitmap == nullptr) {
        throw std::runtime_error("bitmap must not be null");
    }
    validateHotSpot(bitmap, hotSpotX, hotSpotY);

    PointerIcon* icon = new PointerIcon(TYPE_CUSTOM);
    icon->mBitmap = bitmap;
    icon->mHotSpotX = hotSpotX;
    icon->mHotSpotY = hotSpotY;
    return icon;
}

PointerIcon* PointerIcon::load(Context*ctx,const std::string& resourceId) {
    PointerIcon* icon = new PointerIcon(TYPE_CUSTOM);
    icon->loadResource(ctx, resourceId);
    return icon;
}

PointerIcon* PointerIcon::load(Context* context) {
    if (context == nullptr) {
        throw std::runtime_error("context must not be null");
    }

    if (mSystemIconResourceId.empty() || (mBitmap != nullptr)) {
        return this;
    }

    PointerIcon* result = new PointerIcon(mType);
    result->mSystemIconResourceId = mSystemIconResourceId;
    result->loadResource(context,mSystemIconResourceId);
    return result;
}

int PointerIcon::getType() const{
    return mType;
}

bool PointerIcon::equals(const PointerIcon* otherIcon) {
    if (this == otherIcon) {
        return true;
    }

    if ((otherIcon == nullptr)||(mType != otherIcon->mType)
            || (mSystemIconResourceId != otherIcon->mSystemIconResourceId)) {
        return false;
    }

    if (mSystemIconResourceId.empty() && (mBitmap != otherIcon->mBitmap
            || mHotSpotX != otherIcon->mHotSpotX
            || mHotSpotY != otherIcon->mHotSpotY)) {
        return false;
    }

    return true;
}

Bitmap PointerIcon::getBitmapFromDrawable(BitmapDrawable* bitmapDrawable) {
    Bitmap bitmap = bitmapDrawable->getBitmap();
    const int scaledWidth  = bitmapDrawable->getIntrinsicWidth();
    const int scaledHeight = bitmapDrawable->getIntrinsicHeight();
    if (scaledWidth == bitmap->get_width() && scaledHeight == bitmap->get_height()) {
        return bitmap;
    }

    Rect src ={0, 0, bitmap->get_width(), bitmap->get_height()};
    RectF dst ={0.f, 0.f, float(scaledWidth), float(scaledHeight)};

    Bitmap scaled = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,scaledWidth, scaledHeight);//, bitmap.getConfig());
    Canvas canvas(scaled);
    //canvas.drawBitmap(bitmap, src, dst, paint);
    return scaled;
}

void PointerIcon::loadResource(Context* context, const std::string& resourceId) {
    int type;
    float hotSpotX,hotSpotY;
    XmlPullParser parser(context,resourceId);
    const AttributeSet& attrs = parser;
    while ((type=parser.next()) != XmlPullParser::START_TAG
               && type != XmlPullParser::END_DOCUMENT) {
        //EMPTY
    }

    if (type != XmlPullParser::START_TAG){
       //XmlUtils.beginDocument(parser, "pointer-icon");
    }
    const std::string bitmapRes = attrs.getString("bitmap");
    hotSpotX = attrs.getDimension("hotSpotX", 0);
    hotSpotY = attrs.getDimension("hotSpotY", 0);

    if (bitmapRes.empty()) {
        throw std::logic_error("<pointer-icon> is missing bitmap attribute.");
    }

    Drawable* drawable = nullptr;
    drawable = context->getDrawable(bitmapRes);

    if (dynamic_cast<AnimationDrawable*>(drawable)) {
        // Extract animation frame bitmaps.
        AnimationDrawable* animationDrawable = (AnimationDrawable*) drawable;
        const int frames = animationDrawable->getNumberOfFrames();
        drawable = animationDrawable->getFrame(0);
        if (frames == 1) {
            LOGW("Animation icon with single frame -- simply treating the first frame as a normal bitmap icon.");
        } else {
            // Assumes they have the exact duration.
            mDurationPerFrame = animationDrawable->getDuration(0);
            mBitmapFrames.resize(frames-1);
            const int width = drawable->getIntrinsicWidth();
            const int height = drawable->getIntrinsicHeight();
            for (int i = 1; i < frames; ++i) {
                Drawable* drawableFrame = animationDrawable->getFrame(i);
                if (!(dynamic_cast<BitmapDrawable*>(drawableFrame))) {
                    throw std::logic_error("Frame of an animated pointer icon must refer to a bitmap drawable.");
                }
                if (drawableFrame->getIntrinsicWidth() != width ||
                    drawableFrame->getIntrinsicHeight() != height) {
                    throw std::logic_error("All frames should have the exact same size and share the same hotspot.");
                }
                BitmapDrawable* bitmapDrawableFrame = (BitmapDrawable*) drawableFrame;
                mBitmapFrames[i - 1] = getBitmapFromDrawable(bitmapDrawableFrame);
            }
        }
    }
    if (dynamic_cast<BitmapDrawable*>(drawable)==nullptr) {
        throw std::logic_error("<pointer-icon> bitmap attribute must refer to a bitmap drawable.");
    }

    BitmapDrawable* bitmapDrawable = (BitmapDrawable*) drawable;
    Bitmap bitmap = getBitmapFromDrawable(bitmapDrawable);
    validateHotSpot(bitmap, hotSpotX, hotSpotY);
    // Set the properties now that we have successfully loaded the icon.
    mBitmap = bitmap;
    mHotSpotX = hotSpotX;
    mHotSpotY = hotSpotY;
}

void PointerIcon::validateHotSpot(Bitmap bitmap, float hotSpotX, float hotSpotY) {
    if (hotSpotX < 0 || hotSpotX >= bitmap->get_width()) {
        throw std::runtime_error("x hotspot lies outside of the bitmap area");
    }
    if (hotSpotY < 0 || hotSpotY >= bitmap->get_height()) {
        throw std::runtime_error("y hotspot lies outside of the bitmap area");
    }
}

std::string PointerIcon::getSystemIconTypeIndex(int type) {
    switch (type) {
#if 0
    case TYPE_ARROW:
        return com.android.internal.R.styleable.Pointer_pointerIconArrow;
    case TYPE_SPOT_HOVER:
        return com.android.internal.R.styleable.Pointer_pointerIconSpotHover;
    case TYPE_SPOT_TOUCH:
        return com.android.internal.R.styleable.Pointer_pointerIconSpotTouch;
    case TYPE_SPOT_ANCHOR:
        return com.android.internal.R.styleable.Pointer_pointerIconSpotAnchor;
    case TYPE_HAND:
        return com.android.internal.R.styleable.Pointer_pointerIconHand;
    case TYPE_CONTEXT_MENU:
        return com.android.internal.R.styleable.Pointer_pointerIconContextMenu;
    case TYPE_HELP:
        return com.android.internal.R.styleable.Pointer_pointerIconHelp;
    case TYPE_WAIT:
        return com.android.internal.R.styleable.Pointer_pointerIconWait;
    case TYPE_CELL:
        return com.android.internal.R.styleable.Pointer_pointerIconCell;
    case TYPE_CROSSHAIR:
        return com.android.internal.R.styleable.Pointer_pointerIconCrosshair;
    case TYPE_TEXT:
        return com.android.internal.R.styleable.Pointer_pointerIconText;
    case TYPE_VERTICAL_TEXT:
        return com.android.internal.R.styleable.Pointer_pointerIconVerticalText;
    case TYPE_ALIAS:
        return com.android.internal.R.styleable.Pointer_pointerIconAlias;
    case TYPE_COPY:
        return com.android.internal.R.styleable.Pointer_pointerIconCopy;
    case TYPE_ALL_SCROLL:
        return com.android.internal.R.styleable.Pointer_pointerIconAllScroll;
    case TYPE_NO_DROP:
        return com.android.internal.R.styleable.Pointer_pointerIconNodrop;
    case TYPE_HORIZONTAL_DOUBLE_ARROW:
        return com.android.internal.R.styleable.Pointer_pointerIconHorizontalDoubleArrow;
    case TYPE_VERTICAL_DOUBLE_ARROW:
        return com.android.internal.R.styleable.Pointer_pointerIconVerticalDoubleArrow;
    case TYPE_TOP_RIGHT_DIAGONAL_DOUBLE_ARROW:
        return com.android.internal.R.styleable.
                Pointer_pointerIconTopRightDiagonalDoubleArrow;
    case TYPE_TOP_LEFT_DIAGONAL_DOUBLE_ARROW:
        return com.android.internal.R.styleable.
                Pointer_pointerIconTopLeftDiagonalDoubleArrow;
    case TYPE_ZOOM_IN:
        return com.android.internal.R.styleable.Pointer_pointerIconZoomIn;
    case TYPE_ZOOM_OUT:
        return com.android.internal.R.styleable.Pointer_pointerIconZoomOut;
    case TYPE_GRAB:
        return com.android.internal.R.styleable.Pointer_pointerIconGrab;
    case TYPE_GRABBING:
        return com.android.internal.R.styleable.Pointer_pointerIconGrabbing;
#endif
    default:
        return "";
    }
}

}/*endof namespace*/
