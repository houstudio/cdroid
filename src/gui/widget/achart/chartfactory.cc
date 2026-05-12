#include <widget/achart/chartfactory.h>
namespace cdroid{
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
GraphicalView* ChartFactory::getLineChartView(Context* context,
        const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer) {
    checkParameters(dataset, renderer);
    XYChart* chart = new LineChart(dataset, renderer);
    return new GraphicalView(context, chart);
}

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
GraphicalView* ChartFactory::getCubeLineChartView(Context* context,
        const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, float smoothness) {
    checkParameters(dataset, renderer);
    XYChart* chart = new CubicLineChart(dataset, renderer, smoothness);
    return new GraphicalView(context, chart);
}

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
GraphicalView* ChartFactory::getScatterChartView(Context* context,
        const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer) {
    checkParameters(dataset, renderer);
    XYChart* chart = new ScatterChart(dataset, renderer);
    return new GraphicalView(context, chart);
}

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
GraphicalView* ChartFactory::getBubbleChartView(Context* context,
        const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer) {
    checkParameters(dataset, renderer);
    XYChart* chart = new BubbleChart(dataset, renderer);
    return new GraphicalView(context, chart);
}

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
GraphicalView* ChartFactory::getTimeChartView(Context* context,
        const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, const std::string& format) {
    checkParameters(dataset, renderer);
    TimeChart* chart = new TimeChart(dataset, renderer);
    chart->setDateFormat(format);
    return new GraphicalView(context, chart);
}

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
GraphicalView* ChartFactory::getBarChartView(Context* context,
        const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, BarChart::Type type) {
    checkParameters(dataset, renderer);
    XYChart* chart = new BarChart(dataset, renderer, type);
    return new GraphicalView(context, chart);
}

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
GraphicalView* ChartFactory::getRangeBarChartView(Context* context,
        const std::shared_ptr<XYMultipleSeriesDataset>& dataset, const std::shared_ptr<XYMultipleSeriesRenderer>& renderer, BarChart::Type type) {
    checkParameters(dataset, renderer);
    XYChart* chart = new RangeBarChart(dataset, renderer, type);
    return new GraphicalView(context, chart);
}

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
GraphicalView* ChartFactory::getCombinedXYChartView(Context* context,
        const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer,const std::vector<std::string>& types) {
    if (dataset == nullptr || renderer == nullptr || types.empty()
            || dataset->getSeriesCount() != types.size()) {
        throw std::invalid_argument("Dataset, renderer and types should be not null and the datasets series count should be equal to the types length");
    }
    checkParameters(dataset, renderer);
    CombinedXYChart* chart = new CombinedXYChart(dataset, renderer, types);
    return new GraphicalView(context, chart);
}

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
GraphicalView* ChartFactory::getCombinedTimeChartView(Context* context,
        const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer,const std::vector<std::string>& types) {
    if (dataset == nullptr || renderer == nullptr || types.empty()
            || dataset->getSeriesCount() != types.size()) {
        throw std::invalid_argument("Dataset, renderer and types should be not null and the datasets series count should be equal to the types length");
    }
    checkParameters(dataset, renderer);
    CombinedTimeChart* chart = new CombinedTimeChart(dataset, renderer, types);
    return new GraphicalView(context, chart);
}

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
GraphicalView* ChartFactory::getPieChartView(Context* context,
        const std::shared_ptr<CategorySeries>& dataset,
        const std::shared_ptr<DefaultRenderer>& renderer) {
    checkParameters(dataset, renderer);
    PieChart* chart = new PieChart(dataset, renderer);
    return new GraphicalView(context, chart);
}

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
GraphicalView* ChartFactory::getDialChartView(Context* context,
        const std::shared_ptr<CategorySeries>& dataset,
        const std::shared_ptr<DialRenderer>& renderer) {
    checkParameters(dataset, renderer);
    DialChart* chart = new DialChart(dataset, renderer);
    return new GraphicalView(context, chart);
}

GraphicalView* ChartFactory::getRadarChartView(Context*context,
       const std::shared_ptr<MultipleCategorySeries>& dataset,
        const std::shared_ptr<DefaultRenderer>& renderer){
    checkParameters(dataset, renderer);
    RadarChart*chart = new RadarChart(dataset, renderer);
    return new GraphicalView(context,chart);
}

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
GraphicalView* ChartFactory::getDoughnutChartView(Context* context,
        const std::shared_ptr<MultipleCategorySeries>& dataset,
        const std::shared_ptr<DefaultRenderer>& renderer) {
    checkParameters(dataset, renderer);
    DoughnutChart* chart = new DoughnutChart(dataset, renderer);
    return new GraphicalView(context, chart);
}
/**
 * Checks the validity of the dataset and renderer parameters.
 *
 * @param dataset the multiple series dataset (cannot be null)
 * @param renderer the multiple series renderer (cannot be null)
 * @throws IllegalArgumentException if dataset is null or renderer is null or
 *           if the dataset and the renderer don't include the same number of
 *           series
 */
void ChartFactory::checkParameters(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,
        const std::shared_ptr<XYMultipleSeriesRenderer>& renderer) {
    if (dataset == nullptr || renderer == nullptr
            || dataset->getSeriesCount() != renderer->getSeriesRendererCount()) {
        throw std::invalid_argument(
            "Dataset and renderer should be not null and should have the same number of series");
    }
}

/**
 * Checks the validity of the dataset and renderer parameters.
 *
 * @param dataset the category series dataset (cannot be null)
 * @param renderer the series renderer (cannot be null)
 * @throws IllegalArgumentException if dataset is null or renderer is null or
 *           if the dataset number of items is different than the number of
 *           series renderers
 */
void ChartFactory::checkParameters(const std::shared_ptr<CategorySeries>& dataset,
        const std::shared_ptr<DefaultRenderer>& renderer) {
    if (dataset == nullptr || renderer == nullptr
            || dataset->getItemCount() != renderer->getSeriesRendererCount()) {
        throw std::invalid_argument("Dataset and renderer should be not null and the dataset number of items should be equal to the number of series renderers");
    }
}

/**
 * Checks the validity of the dataset and renderer parameters.
 *
 * @param dataset the category series dataset (cannot be null)
 * @param renderer the series renderer (cannot be null)
 * @throws IllegalArgumentException if dataset is null or renderer is null or
 *           if the dataset number of items is different than the number of
 *           series renderers
 */
void ChartFactory::checkParameters(const std::shared_ptr<MultipleCategorySeries>& dataset, const std::shared_ptr<DefaultRenderer>& renderer) {
    if (dataset == nullptr || renderer == nullptr
            || !checkMultipleSeriesItems(dataset, renderer->getSeriesRendererCount())) {
        throw std::invalid_argument("Titles and values should be not null and the dataset number of items should be equal to the number of series renderers");
    }
}

bool ChartFactory::checkMultipleSeriesItems(const std::shared_ptr<MultipleCategorySeries>& dataset, int value) {
    int count = dataset->getCategoriesCount();
    bool equal = true;
    for (int k = 0; k < count && equal; k++) {
        equal = dataset->getValues(k).size() == dataset->getTitles(k).size();
    }
    return equal;
}
}
