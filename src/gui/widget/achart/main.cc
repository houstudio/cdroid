#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <core/app.h>
#include <core/color.h>
#include <widget/achart/chartfactory.h>
#include <widget/achart/graphicalview.h>
#include <widget/achart/chart/targetrangechart.h>
#include <widget/achart/model/categoryseries.h>
#include <widget/achart/model/multiplecategoryseries.h>
#include <widget/achart/model/xymultipleseriesdataset.h>
#include <widget/achart/model/xyseries.h>
#include <widget/achart/model/xyvalueseries.h>
#include <widget/achart/renderer/defaultrenderer.h>
#include <widget/achart/renderer/dialrenderer.h>
#include <widget/achart/renderer/simpleseriesrenderer.h>
#include <widget/achart/renderer/xymultipleseriesrenderer.h>
#include <widget/achart/renderer/xyseriesrenderer.h>
//#include <widget/achart/tools/movelistener.h>
//#include <widget/achart/tools/panlistener.h>
//#include <widget/achart/tools/zoomevent.h>
//#include <widget/achart/tools/zoomlistener.h>
#include <widget/button.h>
#include <widget/cdwindow.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>

namespace {

constexpr int kChartHeight = 420;
constexpr double kDayMillis = 24.0 * 60.0 * 60.0 * 1000.0;
constexpr double kBaseTimeMillis = 1704067200000.0;

int argb(unsigned int value) {
    return static_cast<int>(value);
}

struct ChartPageSpec {
    std::string title;
    cdroid::GraphicalView* chartView;
    bool showZoomButtons;
};

std::string buildPageLabel(const std::string& title, size_t index, size_t total) {
    return "Current Chart: " + title + " (" + std::to_string(index + 1) + "/" + std::to_string(total) + ")";
}

cdroid::TextView* createInstructionsView() {
    auto* instructions = new cdroid::TextView(-1, cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
    instructions->setText(
        "当前页只显示一个图表，点击 next_button 切换到下一个。\n"
        "折线图检查: tooltip、pan、pinch zoom、标签避让。\n"
        "范围图检查: DragControl 拖动、TargetRange 色带和 target 线。\n"
        "饼图检查: tooltip、动态高亮、平移和缩放。");
    instructions->setTextSize(16);
    instructions->setTextColor(cdroid::Color::WHITE);
    instructions->setPadding(16, 16, 16, 12);
    return instructions;
}

cdroid::GraphicalView* createInteractionChart(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::XYMultipleSeriesDataset>();
    auto renderer = std::make_shared<cdroid::XYMultipleSeriesRenderer>();
    renderer->setChartTitle("Line Chart");
    renderer->setXTitle("Time");
    renderer->setYTitle("Amplitude");
    renderer->setApplyBackgroundColor(true);
    renderer->setBackgroundColor(0xFF0D1B2A);
    renderer->setMarginsColor(0xFF10243C);
    renderer->setAxesColor(0xFFB8C4D1);
    renderer->setLabelsColor(0xFFE8EEF5);
    renderer->setXLabelsColor(0xFFE8EEF5);
    renderer->setYLabelsColor(0, 0xFFE8EEF5);
    renderer->setGridColor(0x335B7288);
    renderer->setShowGrid(true);
    renderer->setShowLegend(true);
    renderer->setClickEnabled(true);
    //renderer->setSelectableBuffer(4);
    renderer->setPanEnabled(true, true);
    renderer->setZoomEnabled(true, true);
    renderer->setPanLimits({0.0, 10.0, -1.4, 1.4});
    renderer->setZoomLimits({0.0, 10.0, -1.4, 1.4});
    renderer->setZoomRate(1.25f);
    renderer->setXLabels(6);
    renderer->setYLabels(6);
    renderer->setPointSize(5);
    renderer->setMargins({24, 60, 28, 24});

    auto sineRenderer = std::make_shared<cdroid::XYSeriesRenderer>();
    sineRenderer->setColor(0xFFFF6B6B);
    sineRenderer->setLineWidth(2.0f);
    sineRenderer->setPointStyle(cdroid::PointStyle::CIRCLE_FILLED);
    sineRenderer->setFillPoints(true);
    sineRenderer->setDisplayChartValues(true);
    sineRenderer->setChartValuesTextSize(13);
    sineRenderer->setChartValuesSpacing(8);
    renderer->addSeriesRenderer(sineRenderer);

    auto cosineRenderer = std::make_shared<cdroid::XYSeriesRenderer>();
    cosineRenderer->setColor(0xFF4ECDC4);
    cosineRenderer->setLineWidth(2.0f);
    cosineRenderer->setPointStyle(cdroid::PointStyle::DIAMOND);
    cosineRenderer->setDisplayChartValues(true);
    cosineRenderer->setChartValuesTextSize(13);
    cosineRenderer->setChartValuesSpacing(10);
    renderer->addSeriesRenderer(cosineRenderer);

    auto sineSeries = std::make_shared<cdroid::XYSeries>("Sine");
    auto cosineSeries = std::make_shared<cdroid::XYSeries>("Cosine");
    for (int i = 0; i <= 20; ++i) {
        const double x = i * 0.5;
        sineSeries->add(x, std::sin(x) * std::exp(-x / 12.0));
        cosineSeries->add(x, std::cos(x * 0.85) * std::exp(-x / 13.0));
    }
    dataset->addSeries(sineSeries);
    dataset->addSeries(cosineSeries);

    auto* chartView = cdroid::ChartFactory::getLineChartView(context, dataset, renderer);
    /*chartView->addZoomListener(
        [&statusListener](const cdroid::GraphicalView::ZoomEvent& event) {
            statusListener.onZoomApplied(event);
        },
        [&statusListener]() {
            statusListener.onZoomReset();
        },
        true, true);
    chartView->addPanListener([&statusListener]() {
        statusListener.onPanApplied();
    });*/
    return chartView;
}

cdroid::GraphicalView* createOverviewChart(cdroid::Context* context) {
#if 10
    auto dataset = std::make_shared<cdroid::XYMultipleSeriesDataset>();
    auto renderer = std::make_shared<cdroid::XYMultipleSeriesRenderer>();
    renderer->setChartTitle("DragControl + TargetRange");
    renderer->setXTitle("Window");
    renderer->setYTitle("Value");
    renderer->setApplyBackgroundColor(true);
    renderer->setBackgroundColor(0xFF132A13);
    renderer->setMarginsColor(0xFF17351A);
    renderer->setAxesColor(0xFFDDE7D6);
    renderer->setLabelsColor(0xFFEAF4E3);
    renderer->setXLabelsColor(0xFFEAF4E3);
    renderer->setYLabelsColor(0, 0xFFEAF4E3);
    renderer->setGridColor(0x334D7C4D);
    renderer->setShowGrid(true);
    renderer->setShowLegend(true);
    renderer->setClickEnabled(true);
    //renderer->setSelectableBuffer(4);
    renderer->setPanEnabled(false, false);
    renderer->setZoomEnabled(false, false);
    renderer->setXAxisMin(0.0);
    renderer->setXAxisMax(10.0);
    renderer->setYAxisMin(-1.2);
    renderer->setYAxisMax(1.2);
    renderer->setInitialRange({0.0, 10.0, -1.2, 1.2});
    renderer->setPanLimits({0.0, 10.0, -1.2, 1.2});
    renderer->setZoomInLimitX(1.0);
    renderer->setXLabels(6);
    renderer->setYLabels(5);
    renderer->setMargins({24, 60, 28, 24});

    auto signalRenderer = std::make_shared<cdroid::XYSeriesRenderer>();
    signalRenderer->setColor(0xFFFFD166);
    signalRenderer->setLineWidth(2.0f);
    signalRenderer->setPointStyle(cdroid::PointStyle::POINT);
    signalRenderer->setDisplayChartValues(true);
    signalRenderer->setChartValuesTextSize(12);
    signalRenderer->setChartValuesSpacing(8);
    renderer->addSeriesRenderer(signalRenderer);

    auto dragRenderer = std::make_shared<cdroid::XYSeriesRenderer>();
    dragRenderer->setColor(0xFFF2F2F2);
    dragRenderer->setLineWidth(1.0f);
    dragRenderer->setPointStyle(cdroid::PointStyle::POINT);
    dragRenderer->setShowLegendItem(true);
    renderer->addSeriesRenderer(dragRenderer);

    auto targetRenderer = std::make_shared<cdroid::XYSeriesRenderer>();
    targetRenderer->setColor(0xFFEF476F);
    targetRenderer->setLineWidth(1.0f);
    targetRenderer->setPointStyle(cdroid::PointStyle::POINT);
    targetRenderer->setShowLegendItem(true);
    renderer->addSeriesRenderer(targetRenderer);

    auto signalSeries = std::make_shared<cdroid::XYSeries>("Signal");
    for (int i = 0; i <= 40; ++i) {
        const double x = i * 0.25;
        signalSeries->add(x, std::sin(x * 0.9) * std::exp(-x / 10.0));
    }
    dataset->addSeries(signalSeries);

    auto dragSeries = std::make_shared<cdroid::XYSeries>("DragControl");
    dragSeries->add(0.0, 0.0);
    dragSeries->add(8.0, 0.0);
    dataset->addSeries(dragSeries);

    auto targetSeries = std::make_shared<cdroid::XYSeries>("TargetRange");
    targetSeries->add(1.0, -0.35);
    targetSeries->add(5.0, 0.15);
    targetSeries->add(9.0, 0.55);
    dataset->addSeries(targetSeries);

    const std::vector<std::string> types = {"Line", "DragControl", cdroid::TargetRangeChart::TYPE};
    auto* chartView = cdroid::ChartFactory::getCombinedXYChartView(context, dataset, renderer, types);
    /*chartView->addMoveListener([&statusListener]() {
        statusListener.onMoveApplied();
    });*/
    return chartView;
#else
    return nullptr;
#endif
}

cdroid::GraphicalView* createPieChart(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::CategorySeries>("Resource Mix");
    dataset->add("CPU", 35.0);
    dataset->add("Memory", 25.0);
    dataset->add("I/O", 15.0);
    dataset->add("Network", 10.0);
    dataset->add("Idle", 15.0);

    auto renderer = std::make_shared<cdroid::DefaultRenderer>();
    renderer->setChartTitle("Pie Chart");
    renderer->setChartTitleTextSize(18);
    renderer->setApplyBackgroundColor(true);
    renderer->setBackgroundColor(0xFF1F1630);
    renderer->setLabelsColor(0xFFF6F1FF);
    renderer->setShowLegend(true);
    renderer->setFitLegend(true);
    renderer->setLegendTextSize(14);
    renderer->setLabelsTextSize(13);
    renderer->setDisplayValues(true);
    renderer->setClickEnabled(true);
    renderer->setPanEnabled(true);
    renderer->setZoomEnabled(true);
    renderer->setZoomRate(1.2f);
    renderer->setShowAxes(false);
    renderer->setShowGrid(false);
    renderer->setMargins({20, 20, 20, 20});
    renderer->setStartAngle(25.0f);
    renderer->setScale(1.1f);

    const std::vector<int> colors = {
        static_cast<int>(0xFFFF6B6B),
        static_cast<int>(0xFFFFD166),
        static_cast<int>(0xFF06D6A0),
        static_cast<int>(0xFF118AB2),
        static_cast<int>(0xFF9C89B8)
    };

    for (size_t i = 0; i < colors.size(); ++i) {
        auto sliceRenderer = std::make_shared<cdroid::SimpleSeriesRenderer>();
        sliceRenderer->setColor(colors[i]);
        sliceRenderer->setShowLegendItem(true);
        if (i == 1) {
            sliceRenderer->setHighlighted(true);
        }
        renderer->addSeriesRenderer(sliceRenderer);
    }

    auto* chartView = cdroid::ChartFactory::getPieChartView(context, dataset, renderer);

    /*chartView->addZoomListener(
        [&statusListener](const cdroid::GraphicalView::ZoomEvent& event) {
            statusListener.onZoomApplied(event);
        },
        [&statusListener]() {
            statusListener.onZoomReset();
        },
        true, true);
    chartView->addPanListener([&statusListener]() {
        statusListener.onPanApplied();
    });*/
    return chartView;
}

cdroid::LinearLayout* createZoomButtons(const std::string& prefix, cdroid::GraphicalView& chartView) {
    auto* row = new cdroid::LinearLayout(0, 0,
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
    row->setOrientation(cdroid::LinearLayout::HORIZONTAL);

    auto makeButton = [&](const std::string& text, const cdroid::View::OnClickListener& onClick) {
        auto* button = new cdroid::Button(text, 0, cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
        button->setOnClickListener(onClick);
        row->addView(button, new cdroid::LinearLayout::LayoutParams(
            0, cdroid::ViewGroup::LayoutParams::WRAP_CONTENT, 1.0f));
    };
    makeButton("Zoom In", [&](cdroid::View&) {
        chartView.zoomIn();
        //statusListener.note(prefix + " Zoom In button");
    });
    makeButton("Zoom Out", [&](cdroid::View&) {
        chartView.zoomOut();
        //statusListener.note(prefix + " Zoom Out button");
    });
    makeButton("Zoom Reset", [&](cdroid::View&) {
        chartView.zoomReset();
        //statusListener.note(prefix + " Zoom Reset button");
    });
    return row;
}

cdroid::LinearLayout* createChartPage(const std::string& pageTitle, cdroid::GraphicalView& chartView, bool showZoomButtons) {
    auto* page = new cdroid::LinearLayout(0, 0,
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
    page->setOrientation(cdroid::LinearLayout::VERTICAL);

    auto* titleView = new cdroid::TextView(-1, cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
    titleView->setText(pageTitle);
    titleView->setTextSize(18);
    titleView->setTextColor(0xFFF5F7FA);
    titleView->setPadding(16, 8, 16, 8);
    page->addView(titleView, new cdroid::LinearLayout::LayoutParams(
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT));

    if (showZoomButtons) {
        page->addView(createZoomButtons(pageTitle, chartView),
            new cdroid::LinearLayout::LayoutParams(
                cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
                cdroid::ViewGroup::LayoutParams::WRAP_CONTENT));
    }

    page->addView(&chartView, new cdroid::LinearLayout::LayoutParams(
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT, 420));
    return page;
}

cdroid::TextView* createStatusView() {
    auto* statusView = new cdroid::TextView(-1, cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
    statusView->setText("Status: waiting for pan / zoom / move / click verification");
    statusView->setTextSize(15);
    statusView->setTextColor(0xFFFFF3B0);
    statusView->setPadding(16, 10, 16, 16);
    return statusView;
}

cdroid::TextView* createInstructionsViewAll(size_t pageCount) {
    auto* instructions = new cdroid::TextView(-1, cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
    instructions->setText(
        "This regression page rotates through all migrated charts, " + std::to_string(pageCount)
            + " pages total. Click next_button to view the next chart.\n"
        "XY charts: Line, Cubic, Scatter, Bubble, Time, Bar, RangeBar, RangeStackedBar.\n"
        "Combined charts: CombinedXY, CombinedTime, DragControl, TargetRange.\n"
        "Round charts: Pie, Doughnut, Dial.");
    instructions->setTextSize(16);
    instructions->setTextColor(cdroid::Color::WHITE);
    instructions->setPadding(16, 16, 16, 12);
    return instructions;
}

void configureCommonXYRenderer(cdroid::XYMultipleSeriesRenderer& renderer, const std::string& title,
        const std::string& xTitle, const std::string& yTitle, int backgroundColor, int marginsColor) {
    renderer.setChartTitle(title);
    renderer.setXTitle(xTitle);
    renderer.setYTitle(yTitle);
    renderer.setApplyBackgroundColor(true);
    renderer.setBackgroundColor(backgroundColor);
    renderer.setMarginsColor(marginsColor);
    renderer.setAxesColor(argb(0xFFD8E2EC));
    renderer.setLabelsColor(argb(0xFFF8FBFF));
    renderer.setXLabelsColor(argb(0xFFF8FBFF));
    renderer.setYLabelsColor(0, argb(0xFFF8FBFF));
    renderer.setGridColor(argb(0x335C728A));
    renderer.setShowGrid(true);
    renderer.setShowLegend(true);
    renderer.setClickEnabled(true);
    //renderer.setSelectableBuffer(4);
    renderer.setPanEnabled(true, true);
    renderer.setZoomEnabled(true, true);
    renderer.setZoomRate(1.25f);
    renderer.setXLabels(6);
    renderer.setYLabels(6);
    renderer.setPointSize(5.0f);
    renderer.setMargins({24, 60, 28, 24});
}

void configureCommonRoundRenderer(cdroid::DefaultRenderer& renderer, const std::string& title,
        int backgroundColor) {
    renderer.setChartTitle(title);
    renderer.setChartTitleTextSize(18);
    renderer.setApplyBackgroundColor(true);
    renderer.setBackgroundColor(backgroundColor);
    renderer.setLabelsColor(argb(0xFFF7F3FF));
    renderer.setAxesColor(argb(0xFFE9E4F8));
    renderer.setShowLegend(true);
    renderer.setFitLegend(true);
    renderer.setLegendTextSize(14);
    renderer.setLabelsTextSize(13);
    renderer.setClickEnabled(true);
    renderer.setPanEnabled(true);
    renderer.setZoomEnabled(true);
    renderer.setZoomRate(1.2f);
    renderer.setShowAxes(false);
    renderer.setShowGrid(false);
    renderer.setMargins({20, 20, 20, 20});
    renderer.setStartAngle(25.0f);
    renderer.setScale(1.05f);
}

std::shared_ptr<cdroid::XYSeriesRenderer> createSeriesRenderer(int color, cdroid::PointStyle pointStyle,
        bool fillPoints, bool displayValues, float lineWidth = 2.0f) {
    auto renderer = std::make_shared<cdroid::XYSeriesRenderer>();
    renderer->setColor(color);
    renderer->setPointStyle(pointStyle);
    renderer->setFillPoints(fillPoints);
    renderer->setLineWidth(lineWidth);
    renderer->setDisplayChartValues(displayValues);
    renderer->setChartValuesTextSize(12);
    renderer->setChartValuesSpacing(8);
    return renderer;
}

void attachPanZoomListeners(cdroid::GraphicalView& chartView) {
    /*chartView.addZoomListener(
        [&statusListener](const cdroid::GraphicalView::ZoomEvent& event) {
            statusListener.onZoomApplied(event);
        },
        [&statusListener]() {
            statusListener.onZoomReset();
        },
        true, true);
    chartView.addPanListener([&statusListener]() {
        //statusListener.onPanApplied();
    });*/
}

cdroid::LinearLayout* createChartPage(const ChartPageSpec& spec) {
    auto* page = new cdroid::LinearLayout(0, 0,
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
    page->setOrientation(cdroid::LinearLayout::VERTICAL);

    auto* titleView = new cdroid::TextView(-1, cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
    titleView->setText(spec.title);
    titleView->setTextSize(18);
    titleView->setTextColor(argb(0xFFF5F7FA));
    titleView->setPadding(16, 8, 16, 8);
    page->addView(titleView, new cdroid::LinearLayout::LayoutParams(
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT));

    if (spec.showZoomButtons) {
        page->addView(createZoomButtons(spec.title, *spec.chartView),
            new cdroid::LinearLayout::LayoutParams(
                cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
                cdroid::ViewGroup::LayoutParams::WRAP_CONTENT));
    }

    page->addView(spec.chartView, new cdroid::LinearLayout::LayoutParams(
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT, kChartHeight));
    return page;
}

ChartPageSpec createCubicChart(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::XYMultipleSeriesDataset>();
    auto renderer = std::make_shared<cdroid::XYMultipleSeriesRenderer>();
    configureCommonXYRenderer(*renderer, "Cubic Chart", "Phase", "Response",
        argb(0xFF162033), argb(0xFF1B2A43));
    renderer->setPanLimits({0.0, 12.0, -1.4, 1.4});
    renderer->setZoomLimits({0.0, 12.0, -1.4, 1.4});

    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFFB5E48C), cdroid::PointStyle::POINT, false, false, 2.2f));
    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFFFFB703), cdroid::PointStyle::POINT, false, false, 2.0f));

    auto smoothSeries = std::make_shared<cdroid::XYSeries>("Smooth");
    auto trendSeries = std::make_shared<cdroid::XYSeries>("Trend");
    for (int i = 0; i <= 24; ++i) {
        const double x = i * 0.5;
        smoothSeries->add(x, 0.75 * std::sin(x * 0.75) + 0.18 * std::cos(x * 1.6));
        trendSeries->add(x, 0.55 * std::cos(x * 0.55) - 0.12 * std::sin(x * 1.2));
    }
    dataset->addSeries(smoothSeries);
    dataset->addSeries(trendSeries);

    auto* chartView = cdroid::ChartFactory::getCubeLineChartView(context, dataset, renderer, 0.25f);
    attachPanZoomListeners(*chartView);
    return {"Cubic Chart", chartView, true};
}

ChartPageSpec createScatterChart(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::XYMultipleSeriesDataset>();
    auto renderer = std::make_shared<cdroid::XYMultipleSeriesRenderer>();
    configureCommonXYRenderer(*renderer, "Scatter Chart", "Latency", "Throughput",
        argb(0xFF17212B), argb(0xFF1B2E3C));
    renderer->setPointSize(6.5f);
    renderer->setPanLimits({0.0, 11.0, 0.0, 12.0});
    renderer->setZoomLimits({0.0, 11.0, 0.0, 12.0});

    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFF90E0EF), cdroid::PointStyle::X, false, false, 1.8f));
    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFFFFAFCC), cdroid::PointStyle::TRIANGLE, true, false, 1.8f));
    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFFFFAFCC), cdroid::PointStyle::POINT, true, false, 1.8f));

    auto batchA = std::make_shared<cdroid::XYSeries>("Batch A");
    auto batchB = std::make_shared<cdroid::XYSeries>("Batch B");
    auto batchC = std::make_shared<cdroid::XYSeries>("Batch C");
    for (int i = 0; i < 10; ++i) {
        const double x = 0.8 + i;
        batchA->add(x, 2.2 + std::fmod(i * 2.7, 6.0));
        batchB->add(x + 0.15, 4.0 + std::fmod(i * 1.9, 5.2));
        batchC->add(x + 0.12, 4.0 + std::fmod(i * 1.9, 4.9));
    }
    dataset->addSeries(batchA);
    dataset->addSeries(batchB);
    dataset->addSeries(batchC);

    auto* chartView = cdroid::ChartFactory::getScatterChartView(context, dataset, renderer);
    attachPanZoomListeners(*chartView);
    return {"Scatter Chart", chartView, true};
}

ChartPageSpec createBubbleChart(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::XYMultipleSeriesDataset>();
    auto renderer = std::make_shared<cdroid::XYMultipleSeriesRenderer>();
    configureCommonXYRenderer(*renderer, "Bubble Chart", "Coverage", "Impact",
        argb(0xFF1C1A2E), argb(0xFF24203A));
    renderer->setPanLimits({0.0, 10.0, 0.0, 10.0});
    renderer->setZoomLimits({0.0, 10.0, 0.0, 10.0});

    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFF80ED99), cdroid::PointStyle::CIRCLE_FILLED, true, false));
    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFFFFD166), cdroid::PointStyle::DIAMOND, true, false));

    auto riskSeries = std::make_shared<cdroid::XYValueSeries>("Risk");
    riskSeries->add(1.2, 7.4, 1.6);
    riskSeries->add(2.5, 5.8, 2.4);
    riskSeries->add(4.0, 8.1, 3.1);
    riskSeries->add(6.0, 6.5, 4.0);

    auto growthSeries = std::make_shared<cdroid::XYValueSeries>("Growth");
    growthSeries->add(3.1, 2.5, 1.4);
    growthSeries->add(5.2, 4.2, 2.0);
    growthSeries->add(7.0, 3.8, 3.2);
    growthSeries->add(8.4, 6.1, 4.6);

    dataset->addSeries(riskSeries);
    dataset->addSeries(growthSeries);

    auto* chartView = cdroid::ChartFactory::getBubbleChartView(context, dataset, renderer);
    attachPanZoomListeners(*chartView);
    return {"Bubble Chart", chartView, true};
}

ChartPageSpec createTimeChart(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::XYMultipleSeriesDataset>();
    auto renderer = std::make_shared<cdroid::XYMultipleSeriesRenderer>();
    configureCommonXYRenderer(*renderer, "Time Chart", "Date", "Load",
        argb(0xFF14213D), argb(0xFF1F2A44));
    renderer->setXRoundedLabels(true);
    renderer->setPanLimits({kBaseTimeMillis, kBaseTimeMillis + kDayMillis * 8.0, 0.0, 12.0});
    renderer->setZoomLimits({kBaseTimeMillis, kBaseTimeMillis + kDayMillis * 8.0, 0.0, 12.0});

    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFFFF6B6B), cdroid::PointStyle::CIRCLE_FILLED, true, false));
    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFF72EFDD), cdroid::PointStyle::SQUARE, true, false));

    auto received = std::make_shared<cdroid::XYSeries>("Requests");
    auto resolved = std::make_shared<cdroid::XYSeries>("Resolved");
    for (int i = 0; i < 8; ++i) {
        const double x = kBaseTimeMillis + i * kDayMillis;
        received->add(x, 4.5 + std::sin(i * 0.75) * 2.1 + i * 0.35);
        resolved->add(x, 3.4 + std::cos(i * 0.65) * 1.8 + i * 0.3);
    }
    dataset->addSeries(received);
    dataset->addSeries(resolved);

    auto* chartView = cdroid::ChartFactory::getTimeChartView(context, dataset, renderer, "MM-dd");
    attachPanZoomListeners(*chartView);
    return {"Time Chart", chartView, true};
}

ChartPageSpec createBarChartPage(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::XYMultipleSeriesDataset>();
    auto renderer = std::make_shared<cdroid::XYMultipleSeriesRenderer>();
    configureCommonXYRenderer(*renderer, "Bar Chart", "Sprint", "Tickets",
        argb(0xFF172A3A), argb(0xFF1B3448));
    renderer->setBarSpacing(0.25);
    renderer->setBarWidth(24.0f);
    renderer->setPanLimits({0.0, 6.0, 0.0, 12.0});
    renderer->setZoomLimits({0.0, 6.0, 0.0, 12.0});

    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFF90BE6D), cdroid::PointStyle::POINT, false, true));
    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFFF94144), cdroid::PointStyle::POINT, false, true));

    auto committed = std::make_shared<cdroid::XYSeries>("Committed");
    auto closed = std::make_shared<cdroid::XYSeries>("Closed");
    for (int i = 1; i <= 5; ++i) {
        committed->add(i, 5 + (i % 3) + i * 0.6);
        closed->add(i, 3 + ((i + 1) % 4) + i * 0.45);
    }
    dataset->addSeries(committed);
    dataset->addSeries(closed);

    auto* chartView = cdroid::ChartFactory::getBarChartView(
        context, dataset, renderer, cdroid::BarChart::Type::DEFAULT);
    attachPanZoomListeners(*chartView);
    return {"Bar Chart", chartView, true};
}

ChartPageSpec createRangeBarChartPage(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::XYMultipleSeriesDataset>();
    auto renderer = std::make_shared<cdroid::XYMultipleSeriesRenderer>();
    configureCommonXYRenderer(*renderer, "RangeBar Chart", "Stage", "Temperature",
        argb(0xFF212529), argb(0xFF2B3035));
    renderer->setBarSpacing(0.3);
    renderer->setBarWidth(22.0f);
    renderer->setPanLimits({0.0, 5.5, -2.0, 12.0});
    renderer->setZoomLimits({0.0, 5.5, -2.0, 12.0});

    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFF4CC9F0), cdroid::PointStyle::POINT, false, true));

    auto series = std::make_shared<cdroid::XYSeries>("Envelope");
    for (int i = 1; i <= 4; ++i) {
        const double x = i;
        const double low = 1.0 + i * 0.8;
        const double high = low + 2.5 + (i % 2);
        series->add(x, low);
        series->add(x, high);
    }
    dataset->addSeries(series);

    auto* chartView = cdroid::ChartFactory::getRangeBarChartView(
        context, dataset, renderer, cdroid::BarChart::Type::DEFAULT);
    attachPanZoomListeners(*chartView);
    return {"RangeBar Chart", chartView, true};
}

ChartPageSpec createRangeStackedBarChartPage(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::XYMultipleSeriesDataset>();
    auto renderer = std::make_shared<cdroid::XYMultipleSeriesRenderer>();
    configureCommonXYRenderer(*renderer, "RangeStackedBar Chart", "Stage", "Budget",
        argb(0xFF1E1E2E), argb(0xFF27293D));
    renderer->setBarSpacing(0.2);
    renderer->setBarWidth(20.0f);
    renderer->setPanLimits({0.0, 5.5, 0.0, 16.0});
    renderer->setZoomLimits({0.0, 5.5, 0.0, 16.0});

    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFF06D6A0), cdroid::PointStyle::POINT, false, true));
    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFFFFB703), cdroid::PointStyle::POINT, false, true));

    auto planned = std::make_shared<cdroid::XYSeries>("Planned");
    auto actual = std::make_shared<cdroid::XYSeries>("Actual");
    for (int i = 1; i <= 4; ++i) {
        const double x = i;
        planned->add(x, 1.0 + i);
        planned->add(x, 3.8 + i * 1.1);
        actual->add(x, 0.6 + i * 0.9);
        actual->add(x, 2.4 + i * 1.0);
    }
    dataset->addSeries(planned);
    dataset->addSeries(actual);

    const std::vector<std::string> types = {"RangeStackedBar", "RangeStackedBar"};
    auto* chartView = cdroid::ChartFactory::getCombinedXYChartView(context, dataset, renderer, types);
    attachPanZoomListeners(*chartView);
    return {"RangeStackedBar Chart", chartView, true};
}

ChartPageSpec createCombinedXYChartPage(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::XYMultipleSeriesDataset>();
    auto renderer = std::make_shared<cdroid::XYMultipleSeriesRenderer>();
    configureCommonXYRenderer(*renderer, "CombinedXY Chart", "Index", "Value",
        argb(0xFF102A43), argb(0xFF16324F));
    renderer->setBarSpacing(0.25);
    renderer->setBarWidth(18.0f);
    renderer->setPanLimits({0.0, 7.5, -1.0, 9.0});
    renderer->setZoomLimits({0.0, 7.5, -1.0, 9.0});

    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFFEF476F), cdroid::PointStyle::CIRCLE_FILLED, true, false));
    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFFFFD166), cdroid::PointStyle::POINT, false, true));
    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFF06D6A0), cdroid::PointStyle::X, false, false));
    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFF4CC9F0), cdroid::PointStyle::DIAMOND, true, false));

    auto lineSeries = std::make_shared<cdroid::XYSeries>("Line");
    auto barSeries = std::make_shared<cdroid::XYSeries>("Bar");
    auto scatterSeries = std::make_shared<cdroid::XYSeries>("Scatter");
    auto bubbleSeries = std::make_shared<cdroid::XYValueSeries>("Bubble");
    for (int i = 1; i <= 6; ++i) {
        lineSeries->add(i, 2.5 + std::sin(i * 0.8) * 1.4);
        barSeries->add(i, 3.0 + (i % 3) * 1.1);
        scatterSeries->add(i + 0.1, 1.2 + std::fmod(i * 1.7, 4.6));
        bubbleSeries->add(i - 0.1, 4.0 + std::cos(i * 0.7) * 1.2, 1.0 + i * 0.4);
    }
    dataset->addSeries(lineSeries);
    dataset->addSeries(barSeries);
    dataset->addSeries(scatterSeries);
    dataset->addSeries(bubbleSeries);

    const std::vector<std::string> types = {"Line", "Bar", "Scatter", "Bubble"};
    auto* chartView = cdroid::ChartFactory::getCombinedXYChartView(context, dataset, renderer, types);
    attachPanZoomListeners(*chartView);
    return {"CombinedXY Chart", chartView, true};
}

ChartPageSpec createCombinedTimeChartPage(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::XYMultipleSeriesDataset>();
    auto renderer = std::make_shared<cdroid::XYMultipleSeriesRenderer>();
    configureCommonXYRenderer(*renderer, "CombinedTime Chart", "Date", "Traffic",
        argb(0xFF0F2027), argb(0xFF203A43));
    renderer->setXRoundedLabels(true);
    renderer->setBarSpacing(0.2);
    renderer->setBarWidth(18.0f);
    renderer->setPanLimits({kBaseTimeMillis, kBaseTimeMillis + kDayMillis * 6.0, 0.0, 12.0});
    renderer->setZoomLimits({kBaseTimeMillis, kBaseTimeMillis + kDayMillis * 6.0, 0.0, 12.0});

    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFF72EFDD), cdroid::PointStyle::CIRCLE_FILLED, true, false));
    renderer->addSeriesRenderer(createSeriesRenderer(argb(0xFFF9844A), cdroid::PointStyle::POINT, false, true));

    auto trend = std::make_shared<cdroid::XYSeries>("Trend");
    auto bursts = std::make_shared<cdroid::XYSeries>("Bursts");
    for (int i = 0; i < 6; ++i) {
        const double x = kBaseTimeMillis + i * kDayMillis;
        trend->add(x, 4.2 + std::sin(i * 0.7) * 1.4 + i * 0.3);
        bursts->add(x, 2.8 + (i % 3) * 1.6);
    }
    dataset->addSeries(trend);
    dataset->addSeries(bursts);

    const std::vector<std::string> types = {"Line", "Bar"};
    auto* chartView = cdroid::ChartFactory::getCombinedTimeChartView(context, dataset, renderer, types);
    attachPanZoomListeners(*chartView);
    return {"CombinedTime Chart", chartView, true};
}

ChartPageSpec createDoughnutChartPage(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::MultipleCategorySeries>("Doughnut Mix");
    dataset->add("Q1", {"Core", "Extension", "Other"}, {48.0, 30.0, 22.0});
    dataset->add("Q2", {"Core", "Extension", "Other"}, {44.0, 35.0, 21.0});
    dataset->add("Q3", {"Core", "Extension", "Other"}, {40.0, 39.0, 21.0});

    auto renderer = std::make_shared<cdroid::DefaultRenderer>();
    configureCommonRoundRenderer(*renderer, "Doughnut Chart", argb(0xFF241734));
    renderer->setDisplayValues(true);
    renderer->setScale(1.08f);

    const std::vector<int> colors = {
        argb(0xFFEF476F),
        argb(0xFFFFD166),
        argb(0xFF06D6A0)
    };
    for (int color : colors) {
        auto sliceRenderer = std::make_shared<cdroid::SimpleSeriesRenderer>();
        sliceRenderer->setColor(color);
        sliceRenderer->setShowLegendItem(true);
        renderer->addSeriesRenderer(sliceRenderer);
    }

    auto* chartView = cdroid::ChartFactory::getDoughnutChartView(context, dataset, renderer);
    attachPanZoomListeners(*chartView);
    return {"Doughnut Chart", chartView, true};
}

ChartPageSpec createRadarChartPage(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::MultipleCategorySeries>("Radar");
    dataset->add("Core",
        {"Quality", "Delivery", "Cost", "Stability", "UX", "Ops"},
        {8.5, 7.2, 6.4, 8.8, 7.5, 6.9});
    dataset->add("Target",
        {"Quality", "Delivery", "Cost", "Stability", "UX", "Ops"},
        {7.0, 8.1, 7.6, 7.4, 8.0, 7.8});
    dataset->add("Stretch",
        {"Quality", "Delivery", "Cost", "Stability", "UX", "Ops"},
        {9.0, 8.6, 6.8, 9.2, 8.7, 8.1});

    auto renderer = std::make_shared<cdroid::DefaultRenderer>();
    configureCommonRoundRenderer(*renderer, "Radar Chart", argb(0xFF14213D));
    renderer->setDisplayValues(false);
    renderer->setScale(1.0f);
    renderer->setMargins({24, 24, 24, 24});

    const std::vector<int> colors = {
        argb(0xFFEF476F),
        argb(0xFFFFD166),
        argb(0xFF06D6A0)
    };
    for (size_t i = 0; i < colors.size(); ++i) {
        std::shared_ptr<cdroid::SimpleSeriesRenderer> seriesRenderer =
            std::make_shared<cdroid::SimpleSeriesRenderer>();
        seriesRenderer->setColor(colors[i]);
        seriesRenderer->setShowLegendItem(true);
        renderer->addSeriesRenderer(seriesRenderer);
    }

    auto* chartView = cdroid::ChartFactory::getRadarChartView(context, dataset, renderer);
    //attachPanZoomListeners(*chartView, statusListener);
    return {"Radar Chart", chartView, true};
}

ChartPageSpec createDialChartPage(cdroid::Context* context) {
    auto dataset = std::make_shared<cdroid::CategorySeries>("Dial");
    dataset->add("Current", 68.0);
    dataset->add("Target", 82.0);
    dataset->add("Target", 135.0);

    auto renderer = std::make_shared<cdroid::DialRenderer>();
    configureCommonRoundRenderer(*renderer, "Dial Chart", argb(0xFF1B263B));
    renderer->setMinValue(0.0);
    renderer->setMaxValue(100.0);
    renderer->setMinorTicksSpacing(5.0);
    renderer->setMajorTicksSpacing(10.0);
    renderer->setAngleMin(330.0);
    renderer->setAngleMax(30.0);
    renderer->setVisualTypes({cdroid::DialRenderer::Type::NEEDLE,
            cdroid::DialRenderer::Type::ARROW,
            cdroid::DialRenderer::Type::NEEDLE});

    auto currentRenderer = std::make_shared<cdroid::SimpleSeriesRenderer>();
    currentRenderer->setColor(argb(0xFF4CC9F0));
    currentRenderer->setShowLegendItem(true);
    renderer->addSeriesRenderer(currentRenderer);

    auto targetRenderer = std::make_shared<cdroid::SimpleSeriesRenderer>();
    targetRenderer->setColor(argb(0xFFFFB703));
    targetRenderer->setShowLegendItem(true);
    renderer->addSeriesRenderer(targetRenderer);

    auto tp = std::make_shared<cdroid::SimpleSeriesRenderer>();
    tp->setColor(argb(0xFFFF0000));
    tp->setShowLegendItem(true);
    renderer->addSeriesRenderer(tp);


    auto chartView = cdroid::ChartFactory::getDialChartView(context, dataset, renderer);
    attachPanZoomListeners(*chartView);
    return {"Dial Chart", chartView, true};
}

std::vector<ChartPageSpec> createChartPages(cdroid::Context* context) {
    std::vector<ChartPageSpec> pages;
    pages.push_back({"Line Chart", createInteractionChart(context), true});
    pages.push_back(createCubicChart(context));
    pages.push_back(createScatterChart(context));
    pages.push_back(createBubbleChart(context));
    pages.push_back(createTimeChart(context));
    pages.push_back(createBarChartPage(context));
    pages.push_back(createRangeBarChartPage(context));
    pages.push_back(createRangeStackedBarChartPage(context));
    pages.push_back(createCombinedXYChartPage(context));
    pages.push_back(createCombinedTimeChartPage(context));
    pages.push_back({"DragControl + TargetRange", createOverviewChart(context), false});
    pages.push_back({"Pie Chart", createPieChart(context), true});
    pages.push_back(createDoughnutChartPage(context));
    pages.push_back(createRadarChartPage(context));
    pages.push_back(createDialChartPage(context));
    return pages;
}

} // namespace
int main(int argc,const char*argv[]){
    cdroid::App app(argc,argv);
    auto* window = new cdroid::Window(0,0,-1,-1);

    auto* root = new cdroid::LinearLayout(0, 0, -1, -1);
    root->setOrientation(cdroid::LinearLayout::VERTICAL);
    root->setBackgroundColor(0xFF101820);

    auto* statusView = createStatusView();
    const std::vector<ChartPageSpec> chartSpecs = createChartPages(&app);

    auto* pageTitle = new cdroid::TextView(-1, cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
    pageTitle->setText(buildPageLabel(chartSpecs.front().title, 0, chartSpecs.size()));
    pageTitle->setTextSize(17);
    pageTitle->setTextColor(argb(0xFFFFF3B0));
    pageTitle->setPadding(16, 6, 16, 10);

    auto* nextButton = new cdroid::Button("next_button", 0,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
    auto* buttonRow = new cdroid::LinearLayout(0, 0,
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
    buttonRow->setOrientation(cdroid::LinearLayout::HORIZONTAL);
    buttonRow->addView(nextButton, new cdroid::LinearLayout::LayoutParams(
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT));

    auto* chartHost = new cdroid::LinearLayout(0, 0,
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT);
    chartHost->setOrientation(cdroid::LinearLayout::VERTICAL);

    std::vector<cdroid::View*> pages;
    std::vector<std::string> pageLabels;
    pages.reserve(chartSpecs.size());
    pageLabels.reserve(chartSpecs.size());

    for (size_t i = 0; i < chartSpecs.size(); ++i) {
        pages.push_back(createChartPage(chartSpecs[i]));
        pageLabels.push_back(buildPageLabel(chartSpecs[i].title, i, chartSpecs.size()));
        pages.back()->setVisibility(i == 0 ? cdroid::View::VISIBLE : cdroid::View::GONE);
        chartHost->addView(pages.back(), new cdroid::LinearLayout::LayoutParams(
            cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
            cdroid::ViewGroup::LayoutParams::WRAP_CONTENT));
    }

    auto currentPageIndex = std::make_shared<size_t>(0);
    nextButton->setOnClickListener([pages, pageLabels, pageTitle, currentPageIndex](cdroid::View&) {
        pages[*currentPageIndex]->setVisibility(cdroid::View::GONE);
        *currentPageIndex = (*currentPageIndex + 1) % pages.size();
        pages[*currentPageIndex]->setVisibility(cdroid::View::VISIBLE);
        pageTitle->setText(pageLabels[*currentPageIndex]);
    });

    root->addView(createInstructionsViewAll(chartSpecs.size()), new cdroid::LinearLayout::LayoutParams(
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT));
    root->addView(pageTitle, new cdroid::LinearLayout::LayoutParams(
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT));
    root->addView(buttonRow, new cdroid::LinearLayout::LayoutParams(
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT));
    root->addView(chartHost, new cdroid::LinearLayout::LayoutParams(
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT, 0, 1.0f));
    root->addView(statusView, new cdroid::LinearLayout::LayoutParams(
        cdroid::ViewGroup::LayoutParams::MATCH_PARENT,
        cdroid::ViewGroup::LayoutParams::WRAP_CONTENT));

    window->addView(root);
    app.exec();
    return 0;
}

