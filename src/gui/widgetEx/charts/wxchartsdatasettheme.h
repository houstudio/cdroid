/*
    Copyright (c) 2019-2024 Xavier Leclercq

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

#ifndef _WX_CHARTS_WXCHARTSDATASETTHEME_H_
#define _WX_CHARTS_WXCHARTSDATASETTHEME_H_

#include "dlimpexp.h"
#include "wxareachartdatasetoptions.h"
#include "wxbarchartdatasetoptions.h"
#include "wxboxplotdatasetoptions.h"
#include "wxcolumnchartdatasetoptions.h"
#include "wxlinechartdatasetoptions.h"
#include "wxstackedbarchartdatasetoptions.h"
#include "wxstackedcolumnchartdatasetoptions.h"
#include <wx/sharedptr.h>

class WXDLLIMPEXP_ISHIKO_CHARTS wxChartsDatasetTheme
{
public:
    wxChartsDatasetTheme();

    std::shared_ptr<wxAreaChartDatasetOptions> GetAreaChartDatasetOptions();
    std::shared_ptr<wxBarChartDatasetOptions> GetBarChartDatasetOptions();
    std::shared_ptr<wxBoxPlotDatasetOptions> GetBoxPlotDatasetOptions();
    std::shared_ptr<wxColumnChartDatasetOptions> GetColumnChartDatasetOptions();
    std::shared_ptr<wxLineChartDatasetOptions> GetLineChartDatasetOptions();
    std::shared_ptr<wxStackedBarChartDatasetOptions> GetStackedBarChartDatasetOptions();
    std::shared_ptr<wxStackedColumnChartDatasetOptions> GetStackedColumnChartDatasetOptions();

    void SetAreaChartDatasetOptions(const wxAreaChartDatasetOptions& options);
    void SetBarChartDatasetOptions(const wxBarChartDatasetOptions& options);
    void SetBoxPlotDatasetOptions(const wxBoxPlotDatasetOptions& options);
    void SetColumnChartDatasetOptions(const wxColumnChartDatasetOptions& options);
    void SetLineChartDatasetOptions(const wxLineChartDatasetOptions& options);
    void SetStackedBarChartDatasetOptions(const wxStackedBarChartDatasetOptions& options);
    void SetStackedColumnChartDatasetOptions(const wxStackedColumnChartDatasetOptions& options);

private:
    std::shared_ptr<wxAreaChartDatasetOptions> m_areaChartDatasetOptions;
    std::shared_ptr<wxBarChartDatasetOptions> m_barChartDatasetOptions;
    std::shared_ptr<wxBoxPlotDatasetOptions> m_boxPlotDatasetOptions;
    std::shared_ptr<wxColumnChartDatasetOptions> m_columnChartDatasetOptions;
    std::shared_ptr<wxLineChartDatasetOptions> m_lineChartDatasetOptions;
    std::shared_ptr<wxStackedBarChartDatasetOptions> m_stackedBarChartDatasetOptions;
    std::shared_ptr<wxStackedColumnChartDatasetOptions> m_stackedColumnChartDatasetOptions;
};

#endif
