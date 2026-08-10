if(ENABLE_WEAR_WIDGETS OR ENABLE_RECYCLERVIEW)

# --- Generated widgetEx styleable header (alongside R.h in widget/) -----------
# Run gen_styleable.py by hand with --custom-ids to assign new custom attr ids
# (append-only, stable); the build uses it read-only here.
add_custom_command(
    OUTPUT  ${PROJECT_SOURCE_DIR}/widget/widgetex_styleable.h
            ${PROJECT_SOURCE_DIR}/widget/widgetex_styleable.cc
    COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/gen_styleable.py
            --attrs ${PROJECT_SOURCE_DIR}/widgetEx/res/values/attrs.xml
            --fw-ids ${CMAKE_SOURCE_DIR}/scripts/framework_attrids.txt
            --custom-ids ${CMAKE_SOURCE_DIR}/scripts/custom_attrids.txt
            --name-map ${CMAKE_SOURCE_DIR}/scripts/widgetex_namemap.txt
            --out-h  ${PROJECT_SOURCE_DIR}/widget/widgetex_styleable.h
            --out-cc ${PROJECT_SOURCE_DIR}/widget/widgetex_styleable.cc
            --guard __WIDGETEX_STYLEABLE_H__ --header widgetex_styleable.h
    DEPENDS ${PROJECT_SOURCE_DIR}/widgetEx/res/values/attrs.xml
            ${CMAKE_SOURCE_DIR}/scripts/framework_attrids.txt
            ${CMAKE_SOURCE_DIR}/scripts/widgetex_namemap.txt
            ${CMAKE_SOURCE_DIR}/scripts/gen_styleable.py
    COMMENT "Generating widget/widgetex_styleable.{h,cc}"
    VERBATIM
)

SET(WIDGETEX_SOURCES
    widget/widgetex_styleable.cc
    widgetEx/recyclerview/viewinfostore.cc
    widgetEx/recyclerview/viewboundscheck.cc
    widgetEx/recyclerview/snaphelper.cc
    widgetEx/recyclerview/simpleitemanimator.cc
    widgetEx/recyclerview/scrollbarhelper.cc
    widgetEx/recyclerview/recyclerview.cc
    widgetEx/recyclerview/pagersnaphelper.cc
    widgetEx/recyclerview/orientationhelper.cc
    widgetEx/recyclerview/opreorderer.cc
    widgetEx/recyclerview/linearsnaphelper.cc
    widgetEx/recyclerview/linearsmoothscroller.cc
    widgetEx/recyclerview/linearlayoutmanager.cc
    widgetEx/recyclerview/itemtouchuiutil.cc
    widgetEx/recyclerview/itemtouchhelper.cc
    widgetEx/recyclerview/gridlayoutmanager.cc
    widgetEx/recyclerview/fastscroller.cc
    widgetEx/recyclerview/divideritemdecoration.cc
    widgetEx/recyclerview/defaultitemanimator.cc
    widgetEx/recyclerview/childhelper.cc
    widgetEx/recyclerview/staggeredgridlayoutmanager.cc
    widgetEx/recyclerview/recyclerviewaccessibilitydelegate.cc
    widgetEx/recyclerview/gapworker.cc
    #widgetEx/recyclerview/carousellayoutmanager.cc
    widgetEx/recyclerview/adapterhelper.cc
    widgetEx/viewgrouputils.cc
)
endif()

if(ENABLE_COORDINATORLAYOUT)
    list(APPEND WIDGETEX_SOURCES
        widgetEx/coordinatorlayout/coordinatorlayout.cc
        widgetEx/coordinatorlayout/hideviewonscrollbehavior.cc
        widgetEx/coordinatorlayout/hidebottomviewonscrollbehavior.cc
    )
endif()

if(ENABLE_FLEXBOXLAYOUT)
    list(APPEND WIDGETEX_SOURCES
        widgetEx/flexbox/flexboxlayout.cc
        widgetEx/flexbox/flexboxhelper.cc
        widgetEx/flexbox/flexboxlayoutmanager.cc
        widgetEx/flexbox/flexboxitemdecoration.cc
    )
endif(ENABLE_FLEXBOXLAYOUT)

include(widgetEx/constraintlayout/constraintlayout.cmake)

list(APPEND WIDGETEX_SOURCES
    widgetEx/viewpager2.cc
    widgetEx/tablayoutmediator.cc
    widgetEx/scrolleventadapter.cc
    widgetEx/plotview.cc
    widgetEx/fakedrag.cc
    #widgetEx/mathglview.cc
)

if(ENABLE_LOTTIE)
    list(APPEND WIDGETEX_SOURCES widgetEx/rlottieview.cc)
endif(ENABLE_LOTTIE)

if(ENABLE_QRCODE)
    list(APPEND WIDGETEX_SOURCES
        widgetEx/qrcodegen.cc
        widgetEx/qrcodeview.cc)
endif(ENABLE_QRCODE)

if(ENABLE_BARCODE)
    list(APPEND WIDGETEX_SOURCES widgetEx/barcodeview.cc)
endif(ENABLE_BARCODE)

if(ENABLE_WEARABLE_WIDGETS)
    include(widgetEx/wear/wear.cmake)
endif()
