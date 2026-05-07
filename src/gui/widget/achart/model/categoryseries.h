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
#ifndef __CATEGORY_SERIES_H__
#define __CATEGORY_SERIES_H__
#include <widget/achart/model/xyseries.h>
namespace cdroid{
/**
 * A series for the category charts like the pie ones.
 */
class CategorySeries {
private:
    /** The series title. */
    std::string mTitle;
    /** The series categories. */
    std::vector<std::string> mCategories;
    /** The series values. */
    std::vector<double> mValues ;
public:
    /**
     * Builds a new category series.
     *
     * @param title the series title
     */
    CategorySeries(const std::string& title) {
        mTitle = title;
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
     * Adds a new value to the series
     *
     * @param value the new value
     */
    void add(double value) {
        add(mCategories.size() + "", value);
    }

    /**
     * Adds a new value to the series.
     *
     * @param category the category
     * @param value the new value
     */
    void add(const std::string& category, double value) {
        mCategories.push_back(category);
        mValues.push_back(value);
    }

    /**
     * Replaces the value at the specific index in the series.
     *
     * @param index the index in the series
     * @param category the category
     * @param value the new value
     */
    void set(int index, const std::string& category, double value) {
        mCategories[index]= category;
        mValues[index]=value;
    }

    /**
     * Removes an existing value from the series.
     *
     * @param index the index in the series of the value to remove
     */
    void remove(int index) {
        mCategories.erase(mCategories.begin()+index);
        mValues.erase(mValues.begin()+index);
    }

    /**
     * Removes all the existing values from the series.
     */
    void clear() {
        mCategories.clear();
        mValues.clear();
    }

    /**
     * Returns the value at the specified index.
     *
     * @param index the index
     * @return the value at the index
     */
    double getValue(int index) const{
        return mValues.at(index);
    }

    /**
     * Returns the category name at the specified index.
     *
     * @param index the index
     * @return the category name at the index
     */
    std::string getCategory(int index) const{
        return mCategories.at(index);
    }

    /**
     * Returns the series item count.
     *
     * @return the series item count
     */
    int getItemCount() const{
        return mCategories.size();
    }

    /**
     * Transforms the category series to an XY series.
     *
     * @return the XY series
     */
    XYSeries* toXYSeries() const{
        XYSeries* xySeries = new XYSeries(mTitle);
        int k = 0;
        for (double value : mValues) {
            xySeries->add(++k, value);
        }
        return xySeries;
    }
};
}/*endof namespace*/
#endif/*__CATEGORY_SERIES_H__*/
