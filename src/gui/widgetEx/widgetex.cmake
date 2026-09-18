if(ENABLE_WEAR_WIDGETS OR ENABLE_RECYCLERVIEW)

# --- Generated widgetEx styleable header (alongside R.h in widget/) -----------
# widgetEx attr ids live in each component's res/values/public.xml (androidx
# per-component structure). Add a new widgetEx attr there with a stable 0x02 id;
# gen_styleable.py reads them, _compile_shared_lib pins them via aapt2 <public>.
#
# attrs.xml is per-component (widgetEx/<comp>/res/values/attrs.xml); --attrs takes
# the comma-joined list and gen_styleable merges all declare-styleables. Add a new
# widgetEx component's attrs to _WIDGETEX_ATTR_FILES (DEPENDS, CMake list) — the CSV
# for --attrs is derived from it.
set(_WIDGETEX_ATTR_FILES
    ${PROJECT_SOURCE_DIR}/widgetEx/coordinatorlayout/res/values/attrs.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/flexbox/res/values/attrs.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/constraintlayout/res/values/attrs.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/recyclerview/res/values/attrs.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/tablayout/res/values/attrs.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/appbar/res/values/attrs.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/res/values/attrs.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/navigationview/res/values/attrs.xml
    ${PROJECT_SOURCE_DIR}/navigation/res/values/attrs.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/wear/res/values/attrs.xml)
string(REPLACE ";" "," _WIDGETEX_ATTRS_CSV "${_WIDGETEX_ATTR_FILES}")
# Per-component public.xml (sibling of each attrs.xml) — gen_styleable reads the
# stable 0x02 attr ids from these (androidx per-component source of truth).
set(_WIDGETEX_PUBLIC_FILES
    ${PROJECT_SOURCE_DIR}/widgetEx/coordinatorlayout/res/values/public.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/flexbox/res/values/public.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/constraintlayout/res/values/public.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/recyclerview/res/values/public.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/tablayout/res/values/public.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/appbar/res/values/public.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/res/values/public.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/navigationview/res/values/public.xml
    ${PROJECT_SOURCE_DIR}/navigation/res/values/public.xml
    ${PROJECT_SOURCE_DIR}/widgetEx/wear/res/values/public.xml)
add_custom_command(
    OUTPUT  ${PROJECT_SOURCE_DIR}/widgetEx/widgetex_styleable.h
            ${PROJECT_SOURCE_DIR}/widgetEx/widgetex_styleable.cc
    COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/gen_styleable.py
            --attrs ${_WIDGETEX_ATTRS_CSV}
            --fw-ids ${CMAKE_SOURCE_DIR}/scripts/framework_attrids.txt
            --auto-name
            --require-pins
            --out-h  ${PROJECT_SOURCE_DIR}/widgetEx/widgetex_styleable.h
            --out-cc ${PROJECT_SOURCE_DIR}/widgetEx/widgetex_styleable.cc
            --guard __WIDGETEX_STYLEABLE_H__ --header widgetex_styleable.h
    DEPENDS ${_WIDGETEX_ATTR_FILES} ${_WIDGETEX_PUBLIC_FILES}
            ${CMAKE_SOURCE_DIR}/scripts/framework_attrids.txt
            ${CMAKE_SOURCE_DIR}/scripts/gen_styleable.py
    COMMENT "Generating widgetEx/widgetex_styleable.{h,cc}"
    VERBATIM
)

SET(WIDGETEX_SOURCES
    widgetEx/widgetex_styleable.cc
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
    widgetEx/tablayout/tablayout.cc
)
endif()

if(ENABLE_COORDINATORLAYOUT)
    list(APPEND WIDGETEX_SOURCES
        widgetEx/coordinatorlayout/coordinatorlayout.cc
        widgetEx/coordinatorlayout/hideviewonscrollbehavior.cc
        widgetEx/coordinatorlayout/hidebottomviewonscrollbehavior.cc
        widgetEx/appbar/appbarlayout.cc
        widgetEx/appbar/collapsingtexthelper.cc
        widgetEx/appbar/collapsingtoolbarlayout.cc
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
    widgetEx/navigationview/navigationview.cc
    widgetEx/navigationview/navigationmenuview.cc
    widgetEx/navigationview/navigationmenuitemview.cc
    widgetEx/navigationview/navigationmenupresenter.cc
    widgetEx/navigationview/navigationbarview.cc
    widgetEx/navigationview/navigationbarmenubuilder.cc
    widgetEx/navigationview/navigationbaritemview.cc
    widgetEx/navigationview/navigationbarmenuview.cc
    widgetEx/navigationview/navigationbarpresenter.cc
    widgetEx/navigationview/bottomnavigationitemview.cc
    widgetEx/navigationview/bottomnavigationmenuview.cc
    widgetEx/navigationview/bottomnavigationview.cc
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
