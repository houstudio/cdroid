/*
    Copyright (c) 2017-2024 Xavier Leclercq

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

#ifndef _WX_CHARTS_WXCHARTSTHEME_H_
#define _WX_CHARTS_WXCHARTSTHEME_H_

#include "dlimpexp.h"
#include "wxareachartoptions.h"
#include "wxbarchartoptions.h"
#include "wxboxplotoptions.h"
#include "wxbubblechartoptions.h"
#include "wxcandlestickchartoptions.h"
#include "wxcolumnchartoptions.h"
#include "wxdoughnutchartoptions.h"
#include "wxhistogramoptions.h"
#include "wxlinechartoptions.h"
#include "wxmath2dplotoptions.h"
#include "wxohlcchartoptions.h"
#include "wxpiechartoptions.h"
#include "wxpolarareachartoptions.h"
#include "wxradarchartoptions.h"
#include "wxscatterplotoptions.h"
#include "wxstackedbarchartoptions.h"
#include "wxstackedcolumnchartoptions.h"
#include "wxtimeserieschart.h"
#include "wxchartsdatasetid.h"
#include "wxchartsdatasettheme.h"
#include <wx/sharedptr.h>
#include <map>

/// \defgroup themeclasses

/// Represent a wxCharts theme.

/// \ingroup themeclasses
class WXDLLIMPEXP_ISHIKO_CHARTS wxChartsTheme
{
public:
    wxChartsTheme();

    std::shared_ptr<wxAreaChartOptions> GetAreaChartOptions();
    std::shared_ptr<wxBarChartOptions> GetBarChartOptions();
    std::shared_ptr<wxBoxPlotOptions> GetBoxPlotOptions();
    std::shared_ptr<wxBubbleChartOptions> GetBubbleChartOptions();
    std::shared_ptr<wxCandlestickChartOptions> GetCandlestickChartOptions();
    std::shared_ptr<wxColumnChartOptions> GetColumnChartOptions();
    std::shared_ptr<wxDoughnutChartOptions> GetDoughnutChartOptions();
    std::shared_ptr<wxHistogramOptions> GetHistogramOptions();
    std::shared_ptr<wxLineChartOptions> GetLineChartOptions();
    std::shared_ptr<wxMath2DPlotOptions> GetMath2DPlotOptions();
    std::shared_ptr<wxOHLCChartOptions> GetOHLCChartOptions();
    std::shared_ptr<wxPieChartOptions> GetPieChartOptions();
    std::shared_ptr<wxPolarAreaChartOptions> GetPolarAreaChartOptions();
    std::shared_ptr<wxRadarChartOptions> GetRadarChartOptions();
    std::shared_ptr<wxScatterPlotOptions> GetScatterPlotOptions();
    std::shared_ptr<wxStackedBarChartOptions> GetStackedBarChartOptions();
    std::shared_ptr<wxStackedColumnChartOptions> GetStackedColumnChartOptions();
    std::shared_ptr<wxTimeSeriesChartOptions> GetTimeSeriesChartOptions();

    std::shared_ptr<wxChartsDatasetTheme> GetDatasetTheme(const wxChartsDatasetId& id);
    void SetDatasetTheme(const wxChartsDatasetId& id, std::shared_ptr<wxChartsDatasetTheme> theme);

private:
    std::shared_ptr<wxAreaChartOptions> m_areaChartOptions;
    std::shared_ptr<wxBarChartOptions> m_barChartOptions;
    std::shared_ptr<wxBoxPlotOptions> m_boxPlotOptions;
    std::shared_ptr<wxBubbleChartOptions> m_bubbleChartOptions;
    std::shared_ptr<wxCandlestickChartOptions> m_candlestickChartOptions;
    std::shared_ptr<wxColumnChartOptions> m_columnChartOptions;
    std::shared_ptr<wxDoughnutChartOptions> m_doughnutChartOptions;
    std::shared_ptr<wxHistogramOptions> m_histogramOptions;
    std::shared_ptr<wxLineChartOptions> m_lineChartOptions;
    std::shared_ptr<wxMath2DPlotOptions> m_math2DPlotOptions;
    std::shared_ptr<wxOHLCChartOptions> m_ohlcChartOptions;
    std::shared_ptr<wxPieChartOptions> m_pieChartOptions;
    std::shared_ptr<wxPolarAreaChartOptions> m_polarAreaChartOptions;
    std::shared_ptr<wxRadarChartOptions> m_radarChartOptions;
    std::shared_ptr<wxScatterPlotOptions> m_scatterPlotOptions;
    std::shared_ptr<wxStackedBarChartOptions> m_stackedBarChartOptions;
    std::shared_ptr<wxStackedColumnChartOptions> m_stackedColumnChartOptions;
    std::shared_ptr<wxTimeSeriesChartOptions> m_timeSeriesChartOptions;
    std::map<wxChartsDatasetId, std::shared_ptr<wxChartsDatasetTheme>> m_datasetThemes;
};

extern std::shared_ptr<wxChartsTheme> wxChartsDefaultTheme;

#endif
