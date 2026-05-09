/**
 * Copyright (C) 2009 - 2012 SC 4ViewSoft SRL
 * Copyright (C) 2013 Henning Dodenhof
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
#include<widget/achart/chart/targetrangechart.h>
#include<widget/achart/chart/combinedxychart.h>

#include<widget/achart/chart/timechart.h>
#include<widget/achart/chart/rangestackedbarchart.h>
#include<widget/achart/chart/rangebarchart.h>
#include<widget/achart/chart/barchart.h>
#include<widget/achart/chart/bubblechart.h>
#include<widget/achart/chart/dragcontrolchart.h>
#include<widget/achart/chart/scatterchart.h>
#include<widget/achart/chart/cubiclinechart.h>
#include<widget/achart/chart/linechart.h>
namespace cdroid{

//private Class<?>[] xyChartTypes = new Class<?>[] { TimeChart.class, LineChart.class,
//        CubicLineChart.class, BarChart.class, BubbleChart.class, ScatterChart.class,
//        RangeBarChart.class, RangeStackedBarChart.class, DragControlChart.class, TargetRangeChart.class

CombinedXYChart::CombinedXYChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer,
        const std::vector<std::string>& types):XYChart(dataset, renderer){
    size_t length = types.size();
    mCharts.resize(length);
    for (int i = 0; i < length; i++) {
        mCharts[i] = getXYChart(types[i]);
        configureSubChart(mCharts[i], i);
    }
}

CombinedXYChart::~CombinedXYChart(){
    for(auto chart: mCharts){
        delete chart;
    }
     mCharts.clear();
}

std::shared_ptr<XYMultipleSeriesDataset> CombinedXYChart::createSubDataset(int seriesIndex) const {
    auto dataset = std::make_shared<XYMultipleSeriesDataset>();
    dataset->addSeries(mDataset->getSeriesAt(seriesIndex));
    return dataset;
}

std::shared_ptr<XYMultipleSeriesRenderer> CombinedXYChart::createSubRenderer(int seriesIndex) const {
    auto renderer = std::make_shared<XYMultipleSeriesRenderer>();
    renderer->setClickEnabled(mRenderer->isClickEnabled());
    renderer->setBarSpacing(mRenderer->getBarSpacing());
    renderer->setBarWidth(mRenderer->getBarWidth());
    renderer->setPointSize(mRenderer->getPointSize());
    renderer->setSelectableBuffer(mRenderer->getSelectableBuffer());
    renderer->setOrientation(mRenderer->getOrientation());
    renderer->setXLabels(mRenderer->getXLabels());
    renderer->setYLabels(mRenderer->getYLabels());
    renderer->setXRoundedLabels(mRenderer->isXRoundedLabels());
    renderer->setGridColor(mRenderer->getGridColor());
    renderer->setMargins(mRenderer->getMargins());
    renderer->setMarginsColor(mRenderer->getMarginsColor());

    const int scale = mDataset->getSeriesAt(seriesIndex)->getScaleNumber();
    if (mRenderer->isMinXSet(scale)) {
        renderer->setXAxisMin(mRenderer->getXAxisMin(scale));
    }
    if (mRenderer->isMaxXSet(scale)) {
        renderer->setXAxisMax(mRenderer->getXAxisMax(scale));
    }
    if (mRenderer->isMinYSet(scale)) {
        renderer->setYAxisMin(mRenderer->getYAxisMin(scale));
    }
    if (mRenderer->isMaxYSet(scale)) {
        renderer->setYAxisMax(mRenderer->getYAxisMax(scale));
    }
    renderer->addSeriesRenderer(mRenderer->getSeriesRendererAt(seriesIndex));
    return renderer;
}

void CombinedXYChart::configureSubChart(XYChart* chart, int seriesIndex) const {
    chart->setDatasetRenderer(createSubDataset(seriesIndex), createSubRenderer(seriesIndex));
    configureTargetRangeChart(chart, seriesIndex);
}

void CombinedXYChart::configureTargetRangeChart(XYChart* chart, int seriesIndex) const {
    auto* targetRangeChart = dynamic_cast<TargetRangeChart*>(chart);
    if (targetRangeChart == nullptr) {
        return;
    }

    auto series = mDataset->getSeriesAt(seriesIndex);
    if (series == nullptr || series->getItemCount() <= 0) {
        return;
    }

    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::lowest();
    float target = std::numeric_limits<float>::quiet_NaN();

    for (int i = 0; i < series->getItemCount(); ++i) {
        const float value = static_cast<float>(series->getY(i));
        min = std::min(min, value);
        max = std::max(max, value);
    }

    for (int i = 0; i < series->getItemCount(); ++i) {
        const float value = static_cast<float>(series->getY(i));
        if (value != min && value != max) {
            target = value;
            break;
        }
    }
    targetRangeChart->setValues(min, max, target);
}

XYChart* CombinedXYChart::getXYChart(const std::string& type) const{
    if (type == "Time") {
        return new TimeChart();
    }
    if (type == "Line") {
        return new LineChart(mDataset, mRenderer);
    }
    if (type == "Cubic") {
        return new CubicLineChart(mDataset, mRenderer, 0.33f);
    }
    if (type == "Bar") {
        return new BarChart(mDataset, mRenderer, BarChart::Type::DEFAULT);
    }
    if (type == "Bubble") {
        return new BubbleChart(mDataset, mRenderer);
    }
    if (type == "Scatter") {
        return new ScatterChart(mDataset, mRenderer);
    }
    if (type == "RangeBar") {
        return new RangeBarChart(mDataset, mRenderer, BarChart::Type::DEFAULT);
    }
    if (type == "RangeStackedBar") {
        return new RangeStackedBarChart();
    }
    if (type == "DragControl") {
        return new DragControlChart(mDataset, mRenderer);
    }
    if (type == TargetRangeChart::TYPE) {
        return new TargetRangeChart();
    }
    return nullptr;
}

void CombinedXYChart::syncSubChartState(XYChart* chart, int seriesIndex) const {
    chart->setScreenR(getScreenR());
    chart->setSize(m_width, m_height);

    auto series = mDataset->getSeriesAt(seriesIndex);
    if (series == nullptr) {
        return;
    }

    const int parentScale = series->getScaleNumber();
    const std::vector<double> range = getCalcRange(parentScale);
    if (!range.empty()) {
        chart->setCalcRange(range, parentScale);
        chart->setCalcRange(range, 0);
    }
}

void CombinedXYChart::syncSubChartSelection(XYChart* chart, int seriesIndex) const {
#if 0
    const SeriesSelection* selection = getSelection();
    if (selection == nullptr || selection->getSeriesIndex() != seriesIndex) {
        chart->setSelection(nullptr);
        return;
    }

    chart->setSelection(new SeriesSelection(
        0,
        selection->getPointIndex(),
        selection->getXValue(),
        selection->getValue()));
#endif
}

void CombinedXYChart::drawSeries(Canvas& canvas,  Paint& paint,std::vector<float>& points,
        const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex) {
    //mCharts[seriesIndex]->setScreenR(getScreenR());
    //mCharts[seriesIndex]->setCalcRange(getCalcRange(mDataset->getSeriesAt(seriesIndex)->getScaleNumber()), 0);
    mCharts[seriesIndex]->setSize(m_width, m_height);
    syncSubChartState(mCharts[seriesIndex], seriesIndex);
    mCharts[seriesIndex]->drawSeries(canvas, paint, points, seriesRenderer, yAxisValue, 0,startIndex);
}

std::vector<ClickableArea> CombinedXYChart::clickableAreasForPoints(const std::vector<float>& points,
        const std::vector<double>& values, float yAxisValue, int seriesIndex, int startIndex) {
    return mCharts[seriesIndex]->clickableAreasForPoints(points, values, yAxisValue, 0, startIndex);
}

void CombinedXYChart::drawSeries(const std::shared_ptr<XYSeries>& series, Canvas& canvas,  Paint& paint,
        std::vector<float>& pointsList,const std::shared_ptr<XYSeriesRenderer>& seriesRenderer,
        float yAxisValue, int seriesIndex, int orientation,int startIndex) {
    //mCharts[seriesIndex]->setScreenR(getScreenR());
    //mCharts[seriesIndex]->setSize(m_width, m_height);
    //mCharts[seriesIndex]->setCalcRange(getCalcRange(mDataset->getSeriesAt(seriesIndex)->getScaleNumber()), 0);
    syncSubChartState(mCharts[seriesIndex], seriesIndex);
    mCharts[seriesIndex]->drawSeries(series, canvas, paint, pointsList, seriesRenderer, yAxisValue,
                                    0, orientation, startIndex);
}

SeriesSelection* CombinedXYChart::getSeriesAndPointForScreenCoordinate(const PointF& screenPoint) const {
    SeriesSelection* pointSelection = XYChart::getSeriesAndPointForScreenCoordinate(screenPoint);
    if (pointSelection != nullptr || mRenderer == nullptr || mDataset == nullptr || !mRenderer->isClickEnabled()) {
        return pointSelection;
    }

    for (int seriesIndex = static_cast<int>(mCharts.size()) - 1; seriesIndex >= 0; --seriesIndex) {
        XYChart* chart = mCharts[seriesIndex];
        if (chart == nullptr) {
            continue;
        }

        syncSubChartState(chart, seriesIndex);
        SeriesSelection* childSelection = chart->getSeriesAndPointForScreenCoordinate(screenPoint);
        if (childSelection == nullptr) {
            continue;
        }

        SeriesSelection* mappedSelection = new SeriesSelection(
            seriesIndex, childSelection->getPointIndex(),
            childSelection->getXValue(), childSelection->getValue());
        delete childSelection;
        return mappedSelection;
    }

    return nullptr;
}

int CombinedXYChart::getLegendShapeWidth(int seriesIndex) const{
    return mCharts[seriesIndex]->getLegendShapeWidth(0);
}

void CombinedXYChart::drawLegendShape(Canvas& canvas, const std::shared_ptr<SimpleSeriesRenderer>& renderer,
        float x, float y,int seriesIndex,  Paint& paint) {
    mCharts[seriesIndex]->drawLegendShape(canvas, renderer, x, y, 0, paint);
}

std::string CombinedXYChart::getChartType() const{
    return "Combined";
}

std::vector<XYChart*> CombinedXYChart::getCharts() const{
    return mCharts;
}

}
