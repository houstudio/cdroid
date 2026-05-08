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
#ifndef __TIME_SERIES_H__
#define __TIME_SERIES_H__
#include <widget/achart/model/xyseries.h>
namespace cdroid{
/**
 * A series for the date / time charts.
 */
class TimeSeries :public XYSeries {
public:
    /**
     * Builds a new date / time series.
     *
     * @param title the series title
     */
    TimeSeries(const std::string& title):XYSeries(title){ }

    /**
     * Adds a new value to the series.
     *
     * @param x the date / time value for the X axis
     * @param y the value for the Y axis
     */
    void add(Date x, double y) {
        XYSeries::add(x.getTime(), y);
    }

protected:
    double getPadding() const{
        return 1;
    }
};
}/*endof namespace*/
#endif/*__TIME_SERIES_H__*/
