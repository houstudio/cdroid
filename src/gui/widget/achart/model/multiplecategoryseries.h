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
#ifndef __MULTIPLE_CATEGORY_SERIES_H__
#define __MULTIPLE_CATEGORY_SERIES_H__
#include <vector>
#include <string>
namespace cdroid{
/**
 * A series for the multiple category charts like the doughnut.
 */
class MultipleCategorySeries {
private:
    /** The series title. */
    std::string mTitle;
    /** The series local keys. */
    std::vector<std::string> mCategories;
    /** The series name. */
    std::vector<std::vector<std::string>> mTitles;
    /** The series values. */
    std::vector<std::vector<double>> mValues;
public:
    /**
     * Builds a new category series.
     *
     * @param title the series title
     */
    MultipleCategorySeries(const std::string& title) {
        mTitle = title;
    }

    /**
     * Adds a new value to the series
     *
     * @param titles the titles to be used as labels
     * @param values the new value
     */
    void add(const std::vector<std::string>& titles,const std::vector<double>& values) {
        add(mCategories.size() + "", titles, values);
    }

    /**
     * Adds a new value to the series.
     *
     * @param category the category name
     * @param titles the titles to be used as labels
     * @param values the new value
     */
    void add(const std::string& category,const std::vector<std::string>& titles, const std::vector<double>& values) {
        mCategories.push_back(category);
        mTitles.push_back(titles);
        mValues.push_back(values);
    }

    /**
     * Removes an existing value from the series.
     *
     * @param index the index in the series of the value to remove
     */
    void remove(int index) {
        mCategories.erase(mCategories.begin()+index);
        mTitles.erase(mTitles.begin()+index);
        mValues.erase(mValues.begin()+index);
    }

    /**
     * Removes all the existing values from the series.
     */
    void clear() {
        mCategories.clear();
        mTitles.clear();
        mValues.clear();
    }

    /**
     * Returns the values at the specified index.
     *
     * @param index the index
     * @return the value at the index
     */
    std::vector<double> getValues(int index) const{
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
     * Returns the categories count.
     *
     * @return the categories count
     */
    int getCategoriesCount() const{
        return mCategories.size();
    }

    /**
     * Returns the series item count.
     *
     * @param index the index
     * @return the series item count
     */
    int getItemCount(int index) const{
        return mValues.at(index).size();
    }

    /**
     * Returns the series titles.
     *
     * @param index the index
     * @return the series titles
     */
    std::vector<std::string> getTitles(int index) const{
        return mTitles.at(index);
    }

    const std::vector<std::string>& getCategories() const{
        return mCategories;
    }

    const std::vector<std::vector<std::string>>& getTitles() const{
        return mTitles;
    }

    const std::vector<std::vector<double>>& getValues() const{
        return mValues;
    }
    /**
     * Transforms the category series to an XY series.
     *
     * @return the XY series
     */
    XYSeries* toXYSeries() const{
        XYSeries* xySeries = new XYSeries(mTitle);
        return xySeries;
    }
};
}/*endof namespace*/
#endif/*__MULTIPLE_CATEGORY_SERIES_H__*/
