

# --- Generated styleable headers (R.styleable.X[] equivalent) -----------------
# gen_styleable.py emits framework_styleable.{h,cc} into widget/, alongside R.h
# (which CreatePAK generates into widget/R.h). The files are checked in for
# IDE/clangd and regenerated whenever their inputs change. To grow the framework
# styleable set, edit src/gui/res/values/attrs.xml + the include list in
# widget/framework_styleable_include.txt (one styleable name per line).
set(_FW_STYLEABLE_GEN ${CMAKE_SOURCE_DIR}/scripts/gen_styleable.py)
set(_FW_STYLEABLE_INCLUDE ${PROJECT_SOURCE_DIR}/widget/framework_styleable_include.txt)
# Drift tripwire: the 0x010d private-attr ids are ASSIGNED BY aapt2 (not
# pinned), so hand-maintained id tables drift when attrs.xml changes — a
# drifted table makes every styleable array resolve a NEIGHBOR attr (see the
# 2026-08-15 TimePicker legacyLayout->lightZ crash). Before regenerating the
# styleables, cross-check the tables against the compiled framework arsc and
# fail the build on any mismatch. Fix: re-run the table generator or
# `check_attrids.py --fix`.
set(_ATTRIDS_CHECK ${CMAKE_SOURCE_DIR}/scripts/check_attrids.py)
set(_FRAMEWORK_APK ${CMAKE_BINARY_DIR}/framework.apk)
add_custom_command(
    OUTPUT  ${PROJECT_SOURCE_DIR}/widget/framework_styleable.h
            ${PROJECT_SOURCE_DIR}/widget/framework_styleable.cc
    COMMAND ${Python_EXECUTABLE} ${_ATTRIDS_CHECK}
            --framework-apk ${_FRAMEWORK_APK}
            --table ${CMAKE_SOURCE_DIR}/scripts/framework_attrids.txt
            --table ${CMAKE_SOURCE_DIR}/scripts/cdroid_attrids.txt
    COMMAND ${Python_EXECUTABLE} ${_FW_STYLEABLE_GEN}
            --attrs ${PROJECT_SOURCE_DIR}/res/values/attrs.xml,${PROJECT_SOURCE_DIR}/res/values/attrs_cdroid.xml
            --fw-ids ${CMAKE_SOURCE_DIR}/scripts/framework_attrids.txt
            --cdroid-ids ${CMAKE_SOURCE_DIR}/scripts/cdroid_attrids.txt
            --name-map ${CMAKE_SOURCE_DIR}/scripts/framework_namemap.txt
            --include-file ${_FW_STYLEABLE_INCLUDE}
            --out-h  ${PROJECT_SOURCE_DIR}/widget/framework_styleable.h
            --out-cc ${PROJECT_SOURCE_DIR}/widget/framework_styleable.cc
            --guard __FRAMEWORK_STYLEABLE_H__ --header framework_styleable.h
    DEPENDS ${PROJECT_SOURCE_DIR}/res/values/attrs.xml
            ${PROJECT_SOURCE_DIR}/res/values/attrs_cdroid.xml
            ${CMAKE_SOURCE_DIR}/scripts/framework_attrids.txt
            ${CMAKE_SOURCE_DIR}/scripts/cdroid_attrids.txt
            ${CMAKE_SOURCE_DIR}/scripts/framework_namemap.txt
            ${_FW_STYLEABLE_GEN}
            ${_FW_STYLEABLE_INCLUDE}
            ${_ATTRIDS_CHECK}
            ${_FRAMEWORK_APK}
    COMMENT "Checking attr id tables, then generating widget/framework_styleable.{h,cc}"
    VERBATIM
)


list(APPEND WIDGET_SOURCES
    widget/framework_styleable.cc
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
    widget/floatingtoolbar.cc
    widget/localfloatingtoolbarpopup.cc
)

list(APPEND WIDGET_SOURCES
    #widget/autocompletetextview.cc
    widget/nestedscrollview.cc
    widget/differentialmotionflingcontroller.cc
    widget/scrollview.cc
    widget/horizontalscrollview.cc
    widget/progressbar.cc
    widget/absseekbar.cc
    widget/seekbar.cc
    widget/ratingbar.cc
    widget/space.cc
    widget/textview.cc
    widget/editor.cc
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
    widget/tablelayout.cc
    widget/drawerlayout.cc
)

list(APPEND CDROID_SOURCES
    #widget/activitymanager.cc
    #widget/backendcairo.cc
)

if(ENABLE_DAYTIME_WIDGETS)
    list(APPEND WIDGET_SOURCES
        widget/numerictextview.cc
        widget/yearpickerview.cc
        widget/datepicker.cc
        widget/daypickerview.cc
        widget/daypickerviewpager.cc
        widget/daypickerpageradapter.cc
        widget/daypickerspinnerdelegate.cc
        widget/daypickercalendardelegate.cc
        widget/radialtimepickerview.cc
        widget/textinputtimepickerview.cc
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
    widget/toolbarwidgetwrapper.cc
    widget/toolbaractionbar.cc
    widget/toast.cc
)
if(ENABLE_KPLOT)
list(APPEND WIDGET_SOURCES
    widget/plotaxis.cc
    widget/plotobject.cc
    widget/plotpoint.cc
    widget/plotview.cc
)
endif(ENABLE_KPLOT)

list(APPEND WIDGET_SOURCES
    widget/viewanimator.cc
    widget/dialogviewanimator.cc
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
        widget/achart/chart/combinedtimechart.cc
        widget/achart/chart/linechart.cc
        widget/achart/chart/cubiclinechart.cc
        widget/achart/chart/roundchart.cc
        widget/achart/chart/piechart.cc
        widget/achart/chart/dialchart.cc
        widget/achart/chart/barchart.cc
        widget/achart/chart/rangebarchart.cc
        widget/achart/chart/bubblechart.cc
        widget/achart/chart/doughnutchart.cc
        widget/achart/chart/radarchart.cc
        widget/achart/chart/timechart.cc
        widget/achart/chart/dragcontrolchart.cc
        widget/achart/chart/targetrangechart.cc
        widget/achart/chart/rangestackedbarchart.h
        widget/achart/graphicalview.cc
        widget/achart/chartfactory.cc
    )
endif(ENABLE_ACHART)
add_subdirectory(widget/achart)
