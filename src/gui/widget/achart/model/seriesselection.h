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
#ifndef __SERIES_SECTION_H__
#define __SERIES_SECTION_H__
namespace cdroid{
class SeriesSelection {
private:
    int mSeriesIndex;
    int mPointIndex;
    double mXValue;
    double mValue;
public:
    SeriesSelection():mSeriesIndex(-1),mPointIndex(-1){};
    SeriesSelection(int seriesIndex, int pointIndex, double xValue, double value) {
        mSeriesIndex = seriesIndex;
        mPointIndex = pointIndex;
        mXValue = xValue;
        mValue = value;
    }

    int getSeriesIndex() const{
        return mSeriesIndex;
    }

    int getPointIndex() const{
        return mPointIndex;
    }

    double getXValue() const{
        return mXValue;
    }

    double getValue() const{
        return mValue;
    }
};
}/*endof namespace*/
#endif/*__SERIES_SECTION_H__*/
