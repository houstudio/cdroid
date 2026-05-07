#ifndef __XY_SERRIES_H__
#define __XY_SERRIES_H__
#include <widget/achart/util/mathhelper.h>
#include <widget/achart/util/indexxymap.h>
namespace cdroid{
class XYSeries{
private:
    /** The series title. */
    std::string mTitle;
    /** A map to contain values for X and Y axes and index for each bundle */
    IndexXYMap<double, double> mXY;
    /** The minimum value for the X axis. */
    double mMinX = MathHelper::NULL_VALUE;
    /** The maximum value for the X axis. */
    double mMaxX = -MathHelper::NULL_VALUE;
    /** The minimum value for the Y axis. */
    double mMinY = MathHelper::NULL_VALUE;
    /** The maximum value for the Y axis. */
    double mMaxY = -MathHelper::NULL_VALUE;
    /** The scale number for this series. */
    int mScaleNumber=0;
    /** A padding value that will be added when adding values with the same X. */
    static constexpr double PADDING = 0.000000000001;
    /** Contains the annotations. */
    std::vector<std::string> mAnnotations;
    /** A map contain a (x,y) value for each String annotation. */
    IndexXYMap<double, double> mStringXY;
private:
    void initRange() {
        mMinX = MathHelper::NULL_VALUE;
        mMaxX = -MathHelper::NULL_VALUE;
        mMinY = MathHelper::NULL_VALUE;
        mMaxY = -MathHelper::NULL_VALUE;
        int length = getItemCount();
        for (int k = 0; k < length; k++) {
            double x = getX(k);
            double y = getY(k);
            updateRange(x, y);
        }
    }
    void updateRange(double x, double y) {
        mMinX = std::min(mMinX, x);
        mMaxX = std::max(mMaxX, x);
        mMinY = std::min(mMinY, y);
        mMaxY = std::max(mMaxY, y);
    }
protected:
    double getPadding() const{
        return PADDING;
    }
public:
    /**
     * Builds a new XY series.
     *
     * @param title the series title.
     */
    XYSeries(const std::string& title):XYSeries(title, 0){
    }
    virtual ~XYSeries()=default;
    /**
     * Builds a new XY series.
     *
     * @param title the series title.
     * @param scaleNumber the series scale number
     */
    XYSeries(const std::string& title, int scaleNumber) {
        mTitle = title;
        mScaleNumber = scaleNumber;
        initRange();
    }

    int getScaleNumber() const{
        return mScaleNumber;
    }

    /**
     * Returns the series title.
     *
     * @return the series title
     */
    std::string getTitle() const{
        return mTitle;
    }

    /**
     * Sets the series title.
     *
     * @param title the series title
     */
    void setTitle(const std::string& title) {
        mTitle = title;
    }

    /**
     * Adds a new value to the series.
     *
     * @param x the value for the X axis
     * @param y the value for the Y axis
     */
    virtual void add(double x, double y) {
        while (mXY.find(x) != mXY.end()) {
            // add a very small value to x such as data points sharing the same x will
            // still be added
            x += getPadding();
        }
        mXY.put(x, y);
        updateRange(x, y);
    }

    /**
     * Adds a new value to the series at the specified index.
     *
     * @param index the index to be added the data to
     * @param x the value for the X axis
     * @param y the value for the Y axis
     */
    void add(int index, double x, double y) {
        while (mXY.find(x) != mXY.end()) {
            // add a very small value to x such as data points sharing the same x will
            // still be added
            x += getPadding();
        }
        mXY.put(index,x , y);
        updateRange(x, y);
    }

    /**
     * Removes an existing value from the series.
     *
     * @param index the index in the series of the value to remove
     */
    void remove(int index) {
        XYEntry<double, double> removedEntry = mXY.removeByIndex(index);
        double removedX = removedEntry.getKey();
        double removedY = removedEntry.getValue();
        if (removedX == mMinX || removedX == mMaxX || removedY == mMinY || removedY == mMaxY) {
            initRange();
        }
    }

    /**
     * Removes all the existing values from the series.
     */
    void clear() {
        mXY.clear();
        mStringXY.clear();
        initRange();
    }

    /**
     * Returns the X axis value at the specified index.
     *
     * @param index the index
     * @return the X value
     */
    double getX(int index) const{
        return mXY.getXByIndex(index);
    }

    /**
     * Returns the Y axis value at the specified index.
     *
     * @param index the index
     * @return the Y value
     */
    double getY(int index) const{
        return mXY.getYByIndex(index);
    }

    /**
     * Add an String at (x,y) coordinates
     *
     * @param annotation String text
     * @param x
     * @param y
     */
    void addAnnotation(const std::string& annotation, double x, double y) {
        mAnnotations.push_back(annotation);
        mStringXY.insert({x, y});
    }

    /**
     * Remove an String at index
     *
     * @param index
     */
    void removeAnnotation(int index) {
        mAnnotations.erase(mAnnotations.begin()+index);
        mStringXY.removeByIndex(index);
    }

    /**
     * Get X coordinate of the String at index
     *
     * @param index
     * @return
     */
    double getAnnotationX(int index) {
        return mStringXY.getXByIndex(index);
    }

    /**
     * Get Y coordinate of the String at index
     *
     * @param index
     * @return
     */
    double getAnnotationY(int index) const{
        return mStringXY.getYByIndex(index);
    }

    /**
     * Get String count
     *
     * @return
     */
    int getAnnotationCount() const{
        return mAnnotations.size();
    }

    /**
     * Get the String at index
     *
     * @param index
     * @return String
     */
    std::string getAnnotationAt(int index) const{
        return mAnnotations.at(index);
    }

    /**
     * Returns submap of x and y values according to the given start and end
     *
     * @param start start x value
     * @param stop stop x value
     * @param beforeAfterPoints if the points before and after the first and last
     *          visible ones must be displayed
     * @return a submap of x and y values
     */
    std::map<double, double> getRange(double start, double stop,bool beforeAfterPoints) const{
        double actualStart = start;
        double actualStop = stop;
        if (beforeAfterPoints) {
            auto lowerIt = mXY.lower_bound(start);
            if (lowerIt != mXY.begin()) {
                --lowerIt;
                actualStart = lowerIt->first;
            }
            auto upperIt = mXY.upper_bound(stop);
            if (upperIt != mXY.end()) {
                actualStop = upperIt->first;
            } else {
                actualStop = stop;
            }
        } 
        auto first = mXY.lower_bound(actualStart);
        auto last = mXY.lower_bound(actualStop);
        return std::map<double, double>(first, last);
    }

    int getIndexForKey(double key) const{
        return mXY.getIndexForKey(key);
    }

    /**
     * Returns the series item count.
     *
     * @return the series item count
     */
    int getItemCount() const{
        return mXY.size();
    }

    /**
     * Returns the minimum value on the X axis.
     *
     * @return the X axis minimum value
     */
    double getMinX() const{
        return mMinX;
    }

    /**
     * Returns the minimum value on the Y axis.
     *
     * @return the Y axis minimum value
     */
    double getMinY() const{
        return mMinY;
    }

    /**
     * Returns the maximum value on the X axis.
     *
     * @return the X axis maximum value
     */
    double getMaxX() const{
        return mMaxX;
    }

    /**
     * Returns the maximum value on the Y axis.
     *
     * @return the Y axis maximum value
     */
    double getMaxY() const{
        return mMaxY;
    }
};
}/*endof namespace*/
#endif/*__XY_SERRIES_H__*/
