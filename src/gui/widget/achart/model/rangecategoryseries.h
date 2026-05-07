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
#ifndef __RANGE_CATEGORY_SERIES_H__
#define __RANGE_CATEGORY_SERIES_H__
namespace cdroid{
/**
 * A series for the range category charts like the range bar.
 */
class RangeCategorySeries:public CategorySeries {
private:
    /** The series values. */
    std::vector<double> mMaxValues;
public:
    /**
     * Builds a new category series.
     *
     * @param title the series title
     */
    RangeCategorySeries(const std::string& title):CategorySeries(title){
    }
    /**
     * Adds new values to the series
     *
     * @param minValue the new minimum value
     * @param maxValue the new maximum value
     */
    void add(double minValue, double maxValue) {
        CategorySeries::add(minValue);
        mMaxValues.push_back(maxValue);
    }

    /**
     * Adds new values to the series.
     *
     * @param category the category
     * @param minValue the new minimum value
     * @param maxValue the new maximum value
     */
    void add(const std::string& category, double minValue, double maxValue) {
        CategorySeries::add(category, minValue);
        mMaxValues.push_back(maxValue);
    }

    /**
     * Removes existing values from the series.
     *
     * @param index the index in the series of the values to remove
     */
    void remove(int index) {
        CategorySeries::remove(index);
        mMaxValues.erase(mMaxValues.begin()+index);
    }

    /**
     * Removes all the existing values from the series.
     */
    void clear() {
        CategorySeries::clear();
        mMaxValues.clear();
    }

    /**
     * Returns the minimum value at the specified index.
     *
     * @param index the index
     * @return the minimum value at the index
     */
    double getMinimumValue(int index) const{
        return getValue(index);
    }

    /**
     * Returns the maximum value at the specified index.
     *
     * @param index the index
     * @return the maximum value at the index
     */
    double getMaximumValue(int index) const{
        return mMaxValues.at(index);
    }

    /**
     * Transforms the range category series to an XY series.
     *
     * @return the XY series
     */
    XYSeries* toXYSeries() const{
        XYSeries* xySeries = new XYSeries(getTitle());
        int length = getItemCount();
        for (int k = 0; k < length; k++) {
            xySeries->add(k + 1, getMinimumValue(k));
            // the new fast XYSeries implementation doesn't allow 2 values at the same X,
            // so I had to do a hack until I find a better solution
            xySeries->add(k + 1.000001, getMaximumValue(k));
        }
        return xySeries;
    }
};
}/*endof namespace*/
#endif/*__RANGE_CATEGORY_SERIES_H__*/
