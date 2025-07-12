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
#ifndef __CDROID_GESTURE_H__
#define __CDROID_GESTURE_H__
#include <vector>
#include <atomic>
#include <core/rect.h>
#include <core/path.h>
#include <core/parcelable.h>
#include <gesture/gesturestroke.h>

namespace cdroid{
typedef Cairo::RefPtr<Cairo::ImageSurface> Bitmap;
class Gesture:public Parcelable {
private:
    static constexpr long GESTURE_ID_BASE =0x20242024;// SystemClock::currentTimeMillis();
    static constexpr int BITMAP_RENDERING_WIDTH = 2;
    static constexpr bool BITMAP_RENDERING_ANTIALIAS = true;
    static constexpr bool BITMAP_RENDERING_DITHER = true;
    static std::atomic<int> sGestureCount;
    RectF mBoundingBox;
    // the same as its instance ID
    unsigned long mGestureID;
    std::vector<GestureStroke*> mStrokes;
public:
    Gesture();
    ~Gesture();
    Gesture* clone();
    /**
     * @return all the strokes of the gesture
     */
    const std::vector<GestureStroke*>& getStrokes()const;

    /**
     * @return the number of strokes included by this gesture
     */
    int getStrokesCount()const;

    /**
     * Adds a stroke to the gesture.
     *
     * @param stroke
     */
    void addStroke(GestureStroke* stroke);

    /**
     * Calculates the total length of the gesture. When there are multiple strokes in
     * the gesture, this returns the sum of the lengths of all the strokes.
     *
     * @return the length of the gesture
     */
    float getLength() const;

    /**
     * @return the bounding box of the gesture
     */
    RectF getBoundingBox()const;
    cdroid::Path* toPath();
    cdroid::Path* toPath(Path* path);
    cdroid::Path* toPath(int width, int height, int edge, int numSample);
    cdroid::Path* toPath(Path* path, int width, int height, int edge, int numSample);
    /**
     * Sets the id of the gesture.
     *
     * @param id
     */
    void setID(long id);
    /**
     * @return the id of the gesture
     */
    long getID() const;
    /**
     * Creates a bitmap of the gesture with a transparent background.
     *
     * @param width width of the target bitmap
     * @param height height of the target bitmap
     * @param edge the edge
     * @param numSample
     * @param color
     * @return the bitmap
     */
    Bitmap toBitmap(int width, int height, int edge, int numSample, int color);

    /**
     * Creates a bitmap of the gesture with a transparent background.
     *
     * @param width
     * @param height
     * @param inset
     * @param color
     * @return the bitmap
     */
    Bitmap toBitmap(int width, int height, int inset, int color);
    void serialize(std::ostream& out);
    static Gesture* deserialize(std::istream& in);
#if 0
    public static final @android.annotation.NonNull Parcelable.Creator<Gesture> CREATOR = new Parcelable.Creator<Gesture>() {
        public Gesture createFromParcel(Parcel in);
        public Gesture[] newArray(int size);
    };

    public void writeToParcel(Parcel out, int flags);
    public int describeContents();
#endif
};
}/*endof namespace*/
#endif/*__CDROID_GESTURE_H__*/

