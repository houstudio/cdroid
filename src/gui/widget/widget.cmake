
list(APPEND WIDGET_SOURCES
    widget/edgeeffect.cc
    widget/scroller.cc
    widget/fastscroller.cc
    widget/overscroller.cc
    widget/rtlspacinghelper.cc
    widget/viewdraghelper.cc
    widget/nestedscrollinghelper.cc
    widget/scrollbardrawable.cc
    widget/cdwindow.cc
    widget/cardview.cc
)

list(APPEND WIDGET_SOURCES
    #widget/autocompletetextview.cc
    widget/nestedscrollview.cc
    widget/scrollview.cc
    widget/horizontalscrollview.cc
    widget/progressbar.cc
    widget/absseekbar.cc
    widget/seekbar.cc
    widget/ratingbar.cc
    widget/space.cc
    widget/textview.cc
    widget/edittext.cc
    widget/textclock.cc
    #widget/datetimeview.cc
    widget/checkedtextview.cc
    widget/imagebutton.cc
    widget/imageview.cc
    widget/chronometer.cc
    widget/button.cc
    widget/compoundbutton.cc
    widget/togglebutton.cc
    widget/radiogroup.cc
    widget/switch.cc
    widget/analogclock.cc
    widget/numberpicker.cc
    widget/popupwindow.cc
    widget/candidateview.cc
    widget/keyboardview.cc
    widget/viewpager.cc
)

list(APPEND WIDGET_SOURCES
    widget/adapter.cc
    widget/headerviewlistadapter.cc
    widget/autoscrollhelper.cc
    widget/adapterview.cc
    widget/filterable.cc
    widget/abslistview.cc
    widget/recyclebin.cc
    widget/listview.cc
    widget/gridview.cc
)
if(ENABLE_SPINNER)
    list(APPEND WIDGET_SOURCES
        widget/absspinner.cc
        widget/spinner.cc #spinner need AlertDialog
        widget/forwardinglistener.cc
        widget/listpopupwindow.cc
        widget/dropdownlistview.cc
    )
endif(ENABLE_SPINNER)

list(APPEND WIDGET_SOURCES
    widget/absolutelayout.cc
    widget/linearlayout.cc
    widget/framelayout.cc
    widget/relativelayout.cc
    widget/tabwidget.cc
    widget/gridlayout.cc
    widget/tablerow.cc
    widget/tablayout.cc
    widget/tablelayout.cc
    widget/drawerlayout.cc
)

list(APPEND CDROID_SOURCES
    #widget/activitymanager.cc
    #widget/backendcairo.cc
)

if(ENABLE_DAYTIME_WIDGETS)
    list(APPEND WIDGET_SOURCES
        widget/numberictextview.cc
        widget/yearpickerview.cc
        widget/datepicker.cc
        widget/daypickerview.cc
        widget/daypickerviewpager.cc
        widget/daypickerpageradapter.cc
        widget/daypickerspinnerdelegate.cc
        widget/daypickercalendardelegate.cc
        widget/radialtimepickerview.cc
        #widget/textinputtimepickerview.cc
        widget/timepicker.cc
        widget/timepickerclockdelegate.cc
        widget/timepickerspinnerdelegate.cc
        widget/simplemonthview.cc
        widget/explorebytouchhelper.cc
        widget/calendarview.cc
        widget/calendarviewlegacydelegate.cc
        widget/calendarviewmaterialdelegate.cc
    )
endif(ENABLE_DAYTIME_WIDGETS)

list(APPEND WIDGET_SOURCES
    widget/slidingpanelayout.cc
    widget/mediacontroller.cc
    widget/patternlockview.cc

    widget/actionbar.cc
    widget/toolbar.cc
    widget/toast.cc

    #widget/plotaxis.cc
    #widget/plotobject.cc
    #widget/plotpoint.cc
    #widget/plotview.cc
)

list(APPEND WIDGET_SOURCES
    widget/viewanimator.cc
    widget/viewflipper.cc
    widget/viewswitcher.cc
    widget/textswitcher.cc
    widget/imageswitcher.cc
    widget/adapterviewanimator.cc
    widget/adapterviewflipper.cc
    widget/stackview.cc
)
if(ENABLE_ACHART)
    list(APPEND WIDGET_SOURCES
        widget/achart/model/categoryseries.h
        widget/achart/model/multiplecategoryseries.h
        widget/achart/model/rangecategoryseries.h
        widget/achart/model/seriesselection.h
        widget/achart/model/timeseries.h
        widget/achart/model/xymultipleseriesdataset.h
        widget/achart/model/xyseries.h
        widget/achart/model/xyvalueseries.h

        widget/achart/chart/scatterchart.cc
        widget/achart/chart/abstractchart.cc
        widget/achart/chart/xychart.cc
        widget/achart/chart/combinedxychart.cc
        widget/achart/chart/linechart.cc
        widget/achart/chart/cubiclinechart.cc
        widget/achart/chart/roundchart.cc
        widget/achart/chart/piechart.cc
        widget/achart/chart/dialchart.cc
        widget/achart/chart/barchart.cc
        widget/achart/chart/rangebarchart.cc
        widget/achart/chart/bubblechart.cc
        widget/achart/chart/doughnutchart.cc
        widget/achart/chart/dragcontrolchart.cc
        widget/achart/chart/targetrangechart.cc
        widget/achart/chart/rangestackedbarchart.h
        widget/achart/graphicalview.cc
        widget/achart/chartfactory.cc
        widget/achart/touchhandler.cc
        widget/achart/tools/abstracttool.cc
        widget/achart/tools/move.cc
        widget/achart/tools/pan.cc
        widget/achart/tools/zoom.cc
    )
endif(ENABLE_ACHART)

