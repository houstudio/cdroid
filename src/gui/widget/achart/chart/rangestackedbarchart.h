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
#ifndef __RANGE_STACKED_BARCHART_H__
#define __RANGE_STACKED_BARCHART_H__
#include <widget/achart/chart/rangebarchart.h>

namespace cdroid{
class RangeStackedBarChart :public RangeBarChart {
public:
    RangeStackedBarChart() :RangeBarChart(Type::STACKED){
    }

    std::string getChartType() const{
        return "RangeStackedBar";
    }
};
}
#endif/*__RANGE_STACKED_BARCHART_H__*/
