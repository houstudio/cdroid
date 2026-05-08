#ifndef __CHART_FACTORY_H__
#define __CHART_FACTORY_H__
#include <widget/achart/graphicalview.h>
#include <widget/achart/model/xymultipleseriesdataset.h>
#include <widget/achart/model/multiplecategoryseries.h>

#include <widget/achart/chart/barchart.h>
#include <widget/achart/chart/rangebarchart.h>
#include <widget/achart/chart/piechart.h>
#include <widget/achart/chart/dialchart.h>
#include <widget/achart/chart/doughnutchart.h>
#include <widget/achart/chart/combinedxychart.h>
#include <widget/achart/chart/combinedtimechart.h>
#include <widget/achart/chart/linechart.h>
#include <widget/achart/chart/timechart.h>
#include <widget/achart/chart/bubblechart.h>
#include <widget/achart/chart/scatterchart.h>
#include <widget/achart/chart/cubiclinechart.h>
namespace cdroid{
class ChartFactory {
private:
    ChartFactory() =default;
public:
    /**
     * Creates a line chart view.
     *
     * @param context the context
     * @param dataset the multiple series dataset (cannot be null)
     * @param renderer the multiple series renderer (cannot be null)
     * @return a line chart graphical view
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset and the renderer don't include the same number of
     *           series
     */
    static GraphicalView* getLineChartView(Context* context,
            const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer);

    /**
     * Creates a cubic line chart view.
     *
     * @param context the context
     * @param dataset the multiple series dataset (cannot be null)
     * @param renderer the multiple series renderer (cannot be null)
     * @return a line chart graphical view
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset and the renderer don't include the same number of
     *           series
     */
    static GraphicalView* getCubeLineChartView(Context* context,
            const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, float smoothness);

    /**
     * Creates a scatter chart view.
     *
     * @param context the context
     * @param dataset the multiple series dataset (cannot be null)
     * @param renderer the multiple series renderer (cannot be null)
     * @return a scatter chart graphical view
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset and the renderer don't include the same number of
     *           series
     */
    static GraphicalView* getScatterChartView(Context* context,
            const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer);

    /**
     * Creates a bubble chart view.
     *
     * @param context the context
     * @param dataset the multiple series dataset (cannot be null)
     * @param renderer the multiple series renderer (cannot be null)
     * @return a scatter chart graphical view
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset and the renderer don't include the same number of
     *           series
     */
    static GraphicalView* getBubbleChartView(Context* context,
            const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer);

    /**
     * Creates a time chart view.
     *
     * @param context the context
     * @param dataset the multiple series dataset (cannot be null)
     * @param renderer the multiple series renderer (cannot be null)
     * @param format the date format pattern to be used for displaying the X axis
     *          date labels. If null, a default appropriate format will be used.
     * @return a time chart graphical view
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset and the renderer don't include the same number of
     *           series
     */
    static GraphicalView* getTimeChartView(Context* context,
            const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, const std::string& format);

    /**
     * Creates a bar chart view.
     *
     * @param context the context
     * @param dataset the multiple series dataset (cannot be null)
     * @param renderer the multiple series renderer (cannot be null)
     * @param type the bar chart type
     * @return a bar chart graphical view
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset and the renderer don't include the same number of
     *           series
     */
    static GraphicalView* getBarChartView(Context* context,
            const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, BarChart::Type type);

    /**
     * Creates a range bar chart view.
     *
     * @param context the context
     * @param dataset the multiple series dataset (cannot be null)
     * @param renderer the multiple series renderer (cannot be null)
     * @param type the range bar chart type
     * @return a bar chart graphical view
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset and the renderer don't include the same number of
     *           series
     */
    static GraphicalView* getRangeBarChartView(Context* context,
            const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, BarChart::Type type);
    /**
     * Creates a combined XY chart view.
     *
     * @param context the context
     * @param dataset the multiple series dataset (cannot be null)
     * @param renderer the multiple series renderer (cannot be null)
     * @param types the chart types (cannot be null)
     * @return a combined XY chart graphical view
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if a dataset number of items is different than the number of
     *           series renderers or number of chart types
     */
    static GraphicalView* getCombinedXYChartView(Context* context,
            const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer,const std::vector<std::string>& types);
    /**
     * Creates a combined time chart view.
     *
     * @param context the context
     * @param dataset the multiple series dataset (cannot be null)
     * @param renderer the multiple series renderer (cannot be null)
     * @param types the chart types (cannot be null)
     * @return a combined time chart graphical view
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if a dataset number of items is different than the number of
     *           series renderers or number of chart types
     */
    static GraphicalView* getCombinedTimeChartView(Context* context,
            const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer,const std::vector<std::string>& types);

    /**
     * Creates a pie chart view that can be used to start the graphical view
     * activity.
     *
     * @param context the context
     * @param dataset the category series dataset (cannot be null)
     * @param renderer the series renderer (cannot be null)
     * @return a pie chart view
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset number of items is different than the number of
     *           series renderers
     */
    static GraphicalView* getPieChartView(Context* context,
            const std::shared_ptr<CategorySeries>& dataset,
            const std::shared_ptr<DefaultRenderer>& renderer);

    /**
     * Creates a dial chart view that can be used to start the graphical view
     * activity.
     *
     * @param context the context
     * @param dataset the category series dataset (cannot be null)
     * @param renderer the dial renderer (cannot be null)
     * @return a pie chart view
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset number of items is different than the number of
     *           series renderers
     */
    static GraphicalView* getDialChartView(Context* context,
            const std::shared_ptr<CategorySeries>& dataset,
            const std::shared_ptr<DialRenderer>& renderer);

    /**
     * Creates a doughnut chart view that can be used to start the graphical view
     * activity.
     *
     * @param context the context
     * @param dataset the multiple category series dataset (cannot be null)
     * @param renderer the series renderer (cannot be null)
     * @return a pie chart view
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset number of items is different than the number of
     *           series renderers
     */
    static GraphicalView* getDoughnutChartView(Context* context,
            const std::shared_ptr<MultipleCategorySeries>& dataset,
            const std::shared_ptr<DefaultRenderer>& renderer);
    /**
     * Checks the validity of the dataset and renderer parameters.
     *
     * @param dataset the multiple series dataset (cannot be null)
     * @param renderer the multiple series renderer (cannot be null)
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset and the renderer don't include the same number of
     *           series
     */
private:
    static void checkParameters(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
            const std::shared_ptr<XYMultipleSeriesRenderer>& renderer);

    /**
     * Checks the validity of the dataset and renderer parameters.
     *
     * @param dataset the category series dataset (cannot be null)
     * @param renderer the series renderer (cannot be null)
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset number of items is different than the number of
     *           series renderers
     */
    static void checkParameters(const std::shared_ptr<CategorySeries>&dataset,
            const std::shared_ptr<DefaultRenderer>& renderer);

    /**
     * Checks the validity of the dataset and renderer parameters.
     *
     * @param dataset the category series dataset (cannot be null)
     * @param renderer the series renderer (cannot be null)
     * @throws IllegalArgumentException if dataset is null or renderer is null or
     *           if the dataset number of items is different than the number of
     *           series renderers
     */
    static void checkParameters(
            const std::shared_ptr<MultipleCategorySeries>& dataset,
            const std::shared_ptr<DefaultRenderer>& renderer);
    static bool checkMultipleSeriesItems(const std::shared_ptr<MultipleCategorySeries>& dataset, int value);
};
}/*endof namespace*/
#endif/*__CHART_FACTORY_H__*/
