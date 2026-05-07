project(achart C CXX)

include_directories(
    ../../
)

set(ACHART_SRCS
    model/categoryseries.h
    model/multiplecategoryseries.h
    model/rangecategoryseries.h
    model/seriesselection.h
    model/timeseries.h
    model/xymultipleseriesdataset.h
    model/xyseries.h
    model/xyvalueseries.h

    chart/scatterchart.cc
    chart/abstractchart.cc
    chart/xychart.cc
    chart/combinedxychart.cc
    chart/linechart.cc
    chart/cubiclinechart.cc
    chart/roundchart.cc
    chart/piechart.cc
    chart/dialchart.cc
    chart/barchart.cc
    chart/rangebarchart.cc
    chart/bubblechart.cc
    chart/doughnutchart.cc
    chart/dragcontrolchart.cc
    chart/targetrangechart.cc
    chart/rangestackedbarchart.h
    graphicalview.cc
    chartfactory.cc
    touchhandler.cc
    tools/abstracttool.cc
    tools/move.cc
    tools/pan.cc
    tools/zoom.cc
)

add_library(achart STATIC ${ACHART_SRCS})
add_executable(acharttest main.cc)
target_link_libraries(acharttest cdroid achart)
