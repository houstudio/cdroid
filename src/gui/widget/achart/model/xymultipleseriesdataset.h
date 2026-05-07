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
#ifndef __XY_MULTIPLE_SERIES_DATASET_H__
#define __XY_MULTIPLE_SERIES_DATASET_H__
#include <widget/achart/model/xyseries.h>
namespace cdroid{
/**
 * A series that includes 0 to many XYSeries.
 */
class XYMultipleSeriesDataset{
private:
    /** The included series. */
    std::vector<std::shared_ptr<XYSeries>> mSeries;
public:
    /**
     * Adds a new XY series to the list.
     *
     * @param series the XY series to add
     */
    void addSeries(const std::shared_ptr<XYSeries>& series) {
        mSeries.push_back(series);
    }

    /**
     * Adds a new XY series to the list.
     *
     * @param index the index in the series list
     * @param series the XY series to add
     */
    void addSeries(int index, const std::shared_ptr<XYSeries>& series) {
        mSeries.insert(mSeries.begin()+index, series);
    }

    /**
     * Adds all the provided XY series to the list.
     *
     * @param series the XY series to add
     */
    void addAllSeries(const std::vector<std::shared_ptr<XYSeries>>& series) {
        mSeries.insert(mSeries.end(),series.begin(),series.end());
    }

    /**
     * Removes the XY series from the list.
     *
     * @param index the index in the series list of the series to remove
     */
    void removeSeries(int index) {
        mSeries.erase(mSeries.begin()+index);
    }

    /**
     * Removes the XY series from the list.
     *
     * @param series the XY series to be removed
     */
    void removeSeries(const std::shared_ptr<XYSeries>& series) {
        auto it = std::find(mSeries.begin(),mSeries.end(),series);
        if(it!=mSeries.end())mSeries.erase(it);
    }

    /**
     * Removes all the XY series from the list.
     */
    void clear() {
        mSeries.clear();
    }

    /**
     * Returns the XY series at the specified index.
     *
     * @param index the index
     * @return the XY series at the index
     */
    const std::shared_ptr<XYSeries>& getSeriesAt(int index) const{
        return mSeries.at(index);
    }

    /**
     * Returns the XY series count.
     *
     * @return the XY series count
     */
    int getSeriesCount() const{
        return mSeries.size();
    }

    /**
     * Returns an array of the XY series.
     *
     * @return the XY series array
     */
    std::vector<std::shared_ptr<XYSeries>> getSeries() const{
        return mSeries;
    }
};
}/*endof namespace*/
#endif/*__XY_MULTIPLE_SERIES_DATASET_H__*/
