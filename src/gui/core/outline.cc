#include <core/outline.h>
namespace cdroid{

Outline::Outline() {
   mPath = nullptr;
}

/**
 * Constructs an Outline with a copy of the data in src.
 */
Outline::Outline(const Outline& src):Outline(){
    set(src);
}

Outline::~Outline(){
    delete mPath;
}
/**
 * Sets the outline to be empty.
 *
 * @see #isEmpty()
 */
void Outline::setEmpty() {
    if (mPath != nullptr) {
        // rewind here to avoid thrashing the allocations, but could alternately clear ref
        mPath->reset();//rewind();
    }
    mMode = MODE_EMPTY;
    mRect.setEmpty();
    mRadius = RADIUS_UNDEFINED;
}

/**
 * Returns whether the Outline is empty.
 * <p>
 * Outlines are empty when constructed, or if {@link #setEmpty()} is called,
 * until a setter method is called
 *
 * @see #setEmpty()
 */
bool Outline::isEmpty()const {
    return mMode == MODE_EMPTY;
}


/**
 * Returns whether the outline can be used to clip a View.
 * <p>
 * Currently, only Outlines that can be represented as a rectangle, circle,
 * or round rect support clipping.
 *
 * @see android.view.View#setClipToOutline(boolean)
 */
bool Outline::canClip() const{
    return mMode != MODE_CONVEX_PATH;
}

void Outline::setAlpha(float alpha) {
    mAlpha = alpha;
}

/**
 * Returns the alpha represented by the Outline.
 */
float Outline::getAlpha() const{
    return mAlpha;
}

/**
 * Replace the contents of this Outline with the contents of src.
 *
 * @param src Source outline to copy from.
 */
void Outline::set(const Outline& src) {
    mMode = src.mMode;
    if (src.mMode == MODE_CONVEX_PATH) {
        if (mPath == nullptr) {
            mPath = new Path();
        }
        mPath->append_path(*src.mPath);
    }
    mRect = src.mRect;//.set(src.mRect);
    mRadius = src.mRadius;
    mAlpha = src.mAlpha;
}

/**
 * Sets the Outline to the rounded rect defined by the input rect, and
 * corner radius.
 */
void Outline::setRect(int left, int top, int width, int height) {
    setRoundRect(left, top, width, height, 0.0f);
}

/**
 * Convenience for {@link #setRect(int, int, int, int)}
 */
void Outline::setRect(const Rect& rect) {
    setRect(rect.left, rect.top, rect.width, rect.height);
}

/**
 * Sets the Outline to the rounded rect defined by the input rect, and corner radius.
 * <p>
 * Passing a zero radius is equivalent to calling {@link #setRect(int, int, int, int)}
 */
void Outline::setRoundRect(int left, int top, int width, int height, float radius) {
    if (width==0 || height==0) {
        setEmpty();
        return;
    }

    if (mMode == MODE_CONVEX_PATH) {
        // rewind here to avoid thrashing the allocations, but could alternately clear ref
        mPath->reset();//rewind();
    }
    mMode = MODE_ROUND_RECT;
    mRect.set(left, top, width, height);
    mRadius = radius;
}

/**
 * Convenience for {@link #setRoundRect(int, int, int, int, float)}
 */
void Outline::setRoundRect(const Rect& rect, float radius) {
    setRoundRect(rect.left, rect.top, rect.width, rect.height, radius);
}

/**
 * Populates {@code outBounds} with the outline bounds, if set, and returns
 * {@code true}. If no outline bounds are set, or if a path has been set
 * via {@link #setConvexPath(Path)}, returns {@code false}.
 *
 * @param outRect the rect to populate with the outline bounds, if set
 * @return {@code true} if {@code outBounds} was populated with outline
 *         bounds, or {@code false} if no outline bounds are set
 */
bool Outline::getRect(Rect& outRect) const{
    if (mMode != MODE_ROUND_RECT) {
        return false;
    }
    outRect = mRect;//.set(mRect);
    return true;
}

/**
 * Returns the rounded rect radius, if set, or a value less than 0 if a path has
 * been set via {@link #setConvexPath(Path)}. A return value of {@code 0}
 * indicates a non-rounded rect.
 *
 * @return the rounded rect radius, or value < 0
 */
float Outline::getRadius() const{
    return mRadius;
}

/**
 * Sets the outline to the oval defined by input rect.
 */
void Outline::setOval(int left, int top, int width, int height) {
    if (width==0 || height ==0) {
        setEmpty();
        return;
    }

    if (width == height) {
        // represent circle as round rect, for efficiency, and to enable clipping
        setRoundRect(left, top, width, height, width / 2.0f);
        return;
    }

    if (mPath == nullptr) {
        mPath = new Path();
    } else {
        mPath->reset();
    }

    mMode = MODE_CONVEX_PATH;
    mPath->add_oval(left, top, width, height);//, Path.Direction.CW);
    mRect.setEmpty();
    mRadius = RADIUS_UNDEFINED;
}

/**
 * Convenience for {@link #setOval(int, int, int, int)}
 */
void Outline::setOval(const Rect& rect) {
    setOval(rect.left, rect.top, rect.width, rect.height);
}

/**
 * Sets the Constructs an Outline from a
 * {@link android.graphics.Path#isConvex() convex path}.
 */
void Outline::setConvexPath(const Path& convexPath) {
    /*if (convexPath.isEmpty()) {
        setEmpty();
        return;
    }*/

    if (!convexPath.is_convex()) {
        throw std::runtime_error("path must be convex");
    }

    if (mPath == nullptr) {
        mPath = new Path();
    }

    mMode = MODE_CONVEX_PATH;
    mPath->append_path(convexPath);
    mRect.setEmpty();
    mRadius = RADIUS_UNDEFINED;
}

/**
 * Offsets the Outline by (dx,dy)
 */
void Outline::offset(int dx, int dy) {
    if (mMode == MODE_ROUND_RECT) {
        mRect.offset(dx, dy);
    } else if (mMode == MODE_CONVEX_PATH) {
        //mPath.offset(dx, dy);
    }
}
}
