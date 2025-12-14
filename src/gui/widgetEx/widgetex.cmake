if(ENABLE_WEAR_WIDGETS OR ENABLE_RECYCLERVIEW) 
SET(WIDGETEX_SOURCES
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
    list(APPEND WIDGETEX_SOURCES widgetEx/coordinatorlayout.cc)
endif()

if(ENABLE_FLEXBOXLAYOUT)
    list(APPEND WIDGETEX_SOURCES
        widgetEx/flexbox/flexboxlayout.cc
        widgetEx/flexbox/flexboxhelper.cc
    )
endif(ENABLE_FLEXBOXLAYOUT)

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

