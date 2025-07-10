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
#ifndef __OUTLINE_H__
#define __OUTLINE_H__
#include <core/rect.h>
#include <core/path.h>
#include <math.h>
namespace cdroid{
class Outline {
private:
    static constexpr float RADIUS_UNDEFINED = -INFINITY;//Float.NEGATIVE_INFINITY;
public:
    static constexpr int MODE_EMPTY = 0;
    /** @hide */
    static constexpr int MODE_ROUND_RECT = 1;
    /** @hide */
    static constexpr int MODE_CONVEX_PATH = 2;

    int mMode = MODE_EMPTY;

    Path* mPath;

    /** @hide */
    Rect mRect;
    /** @hide */
    float mRadius = RADIUS_UNDEFINED;
    /** @hide */
    float mAlpha;
public:
    /**
     * Constructs an empty Outline. Call one of the setter methods to make
     * the outline valid for use with a View.
     */
    Outline();

    /**
     * Constructs an Outline with a copy of the data in src.
     */
    Outline(const Outline& src);
    ~Outline();
    /**
     * Sets the outline to be empty.
     *
     * @see #isEmpty()
     */
    void setEmpty() ;

    /**
     * Returns whether the Outline is empty.
     * <p>
     * Outlines are empty when constructed, or if {@link #setEmpty()} is called,
     * until a setter method is called
     *
     * @see #setEmpty()
     */
    bool isEmpty()const;

    /**
     * Returns whether the outline can be used to clip a View.
     * <p>
     * Currently, only Outlines that can be represented as a rectangle, circle,
     * or round rect support clipping.
     *
     * @see android.view.View#setClipToOutline(boolean)
     */
    bool canClip() const;

    /**
     * Sets the alpha represented by the Outline - the degree to which the
     * producer is guaranteed to be opaque over the Outline's shape.
     * <p>
     * An alpha value of <code>0.0f</code> either represents completely
     * transparent content, or content that isn't guaranteed to fill the shape
     * it publishes.
     * <p>
     * Content producing a fully opaque (alpha = <code>1.0f</code>) outline is
     * assumed by the drawing system to fully cover content beneath it,
     * meaning content beneath may be optimized away.
     */
    void setAlpha(float alpha);
    /**
     * Returns the alpha represented by the Outline.
     */
    float getAlpha() const;

    /**
     * Replace the contents of this Outline with the contents of src.
     *
     * @param src Source outline to copy from.
     */
    void set(const Outline& src);
    /**
     * Sets the Outline to the rounded rect defined by the input rect, and
     * corner radius.
     */
    void setRect(int left, int top, int width, int height);

    /**
     * Convenience for {@link #setRect(int, int, int, int)}
     */
    void setRect(const Rect& rect);

    /**
     * Sets the Outline to the rounded rect defined by the input rect, and corner radius.
     * <p>
     * Passing a zero radius is equivalent to calling {@link #setRect(int, int, int, int)}
     */
    void setRoundRect(int left, int top, int width, int height, float radius);

    /**
     * Convenience for {@link #setRoundRect(int, int, int, int, float)}
     */
    void setRoundRect(const Rect& rect, float radius);
    /**
     * Populates {@code outBounds} with the outline bounds, if set, and returns
     * {@code true}. If no outline bounds are set, or if a path has been set
     * via {@link #setConvexPath(Path)}, returns {@code false}.
     *
     * @param outRect the rect to populate with the outline bounds, if set
     * @return {@code true} if {@code outBounds} was populated with outline
     *         bounds, or {@code false} if no outline bounds are set
     */
    bool getRect(Rect& outRect) const;
    /**
     * Returns the rounded rect radius, if set, or a value less than 0 if a path has
     * been set via {@link #setConvexPath(Path)}. A return value of {@code 0}
     * indicates a non-rounded rect.
     *
     * @return the rounded rect radius, or value < 0
     */
    float getRadius() const;

    /**
     * Sets the outline to the oval defined by input rect.
     */
    void setOval(int left, int top, int right, int bottom);

    /**
     * Convenience for {@link #setOval(int, int, int, int)}
     */
    void setOval(const Rect& rect);

    /**
     * Sets the Constructs an Outline from a
     * {@link android.graphics.Path#isConvex() convex path}.
     */
    void setConvexPath(const Path& convexPath);
    /**
     * Offsets the Outline by (dx,dy)
     */
    void offset(int dx, int dy);
};
}/*endof namespace*/
#endif
