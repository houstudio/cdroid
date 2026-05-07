/**
 * Copyright (C) 2009 - 2012 SC 4ViewSoft SRL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __XY_VALUE_SERIES_H__
#define __XY_VALUE_SERIES_H__
#include <widget/achart/model/xyseries.h>
namespace cdroid{
/**
 * An extension of the XY series which adds a third dimension. It is used for XY
 * charts like bubble.
 */
class XYValueSeries :public XYSeries {
private:
    /** A list to contain the series values. */
    std::vector<double> mValue;
    /** The minimum value. */
    double mMinValue = MathHelper::NULL_VALUE;
    /** The maximum value. */
    double mMaxValue = -MathHelper::NULL_VALUE;
private:
    void initRange() {
        mMinValue = MathHelper::NULL_VALUE;
        mMaxValue = MathHelper::NULL_VALUE;
        int length = getItemCount();
        for (int k = 0; k < length; k++) {
            updateRange(getValue(k));
        }
    }

    void updateRange(double value) {
        mMinValue = std::min(mMinValue, value);
        mMaxValue = std::max(mMaxValue, value);
    }
public:
    /**
     * Builds a new XY value series.
     *
     * @param title the series title.
     */
    XYValueSeries(const std::string& title):XYSeries(title){
    }

    /**
     * Adds a new value to the series.
     *
     * @param x the value for the X axis
     * @param y the value for the Y axis
     * @param value the value
     */
    void add(double x, double y, double value) {
        XYSeries::add(x, y);
        mValue.push_back(value);
        updateRange(value);
    }

    /**
     * Adds a new value to the series.
     *
     * @param x the value for the X axis
     * @param y the value for the Y axis
     */
    void add(double x, double y) override{
        add(x, y, (double)0);
    }

    /**
     * Removes an existing value from the series.
     *
     * @param index the index in the series of the value to remove
     */
    void remove(int index) {
        XYSeries::remove(index);
        double removedValue = mValue[index];
        mValue.erase(mValue.begin()+index);
        if (removedValue == mMinValue || removedValue == mMaxValue) {
            initRange();
        }
    }

    /**
     * Removes all the values from the series.
     */
    void clear() {
        XYSeries::clear();
        mValue.clear();
        initRange();
    }

    /**
     * Returns the value at the specified index.
     *
     * @param index the index
     * @return the value
     */
    double getValue(int index) const{
        return mValue.at(index);
    }

    /**
     * Returns the minimum value.
     *
     * @return the minimum value
     */
    double getMinValue() const{
        return mMinValue;
    }

    /**
     * Returns the maximum value.
     *
     * @return the maximum value
     */
    double getMaxValue() const{
        return mMaxValue;
    }
};
}/*endof namespace*/
#endif/*__XY_VALUE_SERIES_H__*/
