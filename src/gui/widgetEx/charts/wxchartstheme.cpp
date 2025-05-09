/*
    Copyright (c) 2017-2019 Xavier Leclercq

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

/// @file

#include "wxchartstheme.h"
#include "wxchartspresentationtheme.h"

wxChartsTheme::wxChartsTheme()
    : m_areaChartOptions(new wxAreaChartOptions()),
    m_barChartOptions(new wxBarChartOptions()),
    m_boxPlotOptions(new wxBoxPlotOptions()),
    m_bubbleChartOptions(new wxBubbleChartOptions()),
    m_candlestickChartOptions(new wxCandlestickChartOptions()),
    m_columnChartOptions(new wxColumnChartOptions()),
    m_doughnutChartOptions(new wxDoughnutChartOptions()),
    m_histogramOptions(new wxHistogramOptions()),
    m_lineChartOptions(new wxLineChartOptions()),
    m_math2DPlotOptions(new wxMath2DPlotOptions()),
    m_ohlcChartOptions(new wxOHLCChartOptions()),
    m_pieChartOptions(new wxPieChartOptions()),
    m_polarAreaChartOptions(new wxPolarAreaChartOptions()),
    m_radarChartOptions(new wxRadarChartOptions()),
    m_scatterPlotOptions(new wxScatterPlotOptions()),
    m_stackedBarChartOptions(new wxStackedBarChartOptions()),
    m_stackedColumnChartOptions(new wxStackedColumnChartOptions()),
    m_timeSeriesChartOptions(new wxTimeSeriesChartOptions())
{
}

std::shared_ptr<wxAreaChartOptions> wxChartsTheme::GetAreaChartOptions()
{
    return m_areaChartOptions;
}

std::shared_ptr<wxBarChartOptions> wxChartsTheme::GetBarChartOptions()
{
    return m_barChartOptions;
}

std::shared_ptr<wxBoxPlotOptions> wxChartsTheme::GetBoxPlotOptions()
{
    return m_boxPlotOptions;
}

std::shared_ptr<wxBubbleChartOptions> wxChartsTheme::GetBubbleChartOptions()
{
    return m_bubbleChartOptions;
}

std::shared_ptr<wxCandlestickChartOptions> wxChartsTheme::GetCandlestickChartOptions()
{
    return m_candlestickChartOptions;
}

std::shared_ptr<wxColumnChartOptions> wxChartsTheme::GetColumnChartOptions()
{
    return m_columnChartOptions;
}

std::shared_ptr<wxDoughnutChartOptions> wxChartsTheme::GetDoughnutChartOptions()
{
    return m_doughnutChartOptions;
}

std::shared_ptr<wxHistogramOptions> wxChartsTheme::GetHistogramOptions()
{
    return m_histogramOptions;
}

std::shared_ptr<wxLineChartOptions> wxChartsTheme::GetLineChartOptions()
{
    return m_lineChartOptions;
}

std::shared_ptr<wxMath2DPlotOptions> wxChartsTheme::GetMath2DPlotOptions()
{
    return m_math2DPlotOptions;
}

std::shared_ptr<wxOHLCChartOptions> wxChartsTheme::GetOHLCChartOptions()
{
    return m_ohlcChartOptions;
}

std::shared_ptr<wxPieChartOptions> wxChartsTheme::GetPieChartOptions()
{
    return m_pieChartOptions;
}

std::shared_ptr<wxPolarAreaChartOptions> wxChartsTheme::GetPolarAreaChartOptions()
{
    return m_polarAreaChartOptions;
}

std::shared_ptr<wxRadarChartOptions> wxChartsTheme::GetRadarChartOptions()
{
    return m_radarChartOptions;
}

std::shared_ptr<wxScatterPlotOptions> wxChartsTheme::GetScatterPlotOptions()
{
    return m_scatterPlotOptions;
}

std::shared_ptr<wxStackedBarChartOptions> wxChartsTheme::GetStackedBarChartOptions()
{
    return m_stackedBarChartOptions;
}

std::shared_ptr<wxStackedColumnChartOptions> wxChartsTheme::GetStackedColumnChartOptions()
{
    return m_stackedColumnChartOptions;
}

std::shared_ptr<wxTimeSeriesChartOptions> wxChartsTheme::GetTimeSeriesChartOptions()
{
    return m_timeSeriesChartOptions;
}

std::shared_ptr<wxChartsDatasetTheme> wxChartsTheme::GetDatasetTheme(const wxChartsDatasetId& id)
{
    return m_datasetThemes[id];
}

void wxChartsTheme::SetDatasetTheme(const wxChartsDatasetId& id, std::shared_ptr<wxChartsDatasetTheme> theme)
{
    m_datasetThemes[id] = theme;
}

// By default the default theme is actually the wxChartsPresentationTheme
std::shared_ptr<wxChartsTheme> wxChartsDefaultTheme(new wxChartsPresentationTheme());
