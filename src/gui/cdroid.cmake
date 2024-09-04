
list(APPEND CDROID_SOURCES
    view/abssavedstate.cc
    view/choreographer.cc
    view/focusfinder.cc
    view/gravity.cc
    view/handleractionqueue.cc
    view/inputevent.cc
    view/inputeventconsistencyverifier.cc
    view/keyevent.cc
    view/layoutinflater.cc
    view/layoutparams.cc
    view/motionevent.cc
    view/rendernode.cc
    view/roundscrollbarrenderer.cc
    view/soundeffectconstants.cc
    view/velocitytracker.cc
    view/view.cc
    view/viewconfiguration.cc
    view/viewgroup.cc
    view/viewoverlay.cc
    view/viewpropertyanimator.cc
    view/viewstub.cc
    view/viewtreeobserver.cc
    view/gesturedetector.cc
    view/scalegesturedetector.cc
    #view/menu.cc
)

list(APPEND CDROID_SOURCES
    widget/edgeeffect.cc
    widget/scroller.cc
    widget/fastscroller.cc
    widget/overscroller.cc
    widget/rtlspacinghelper.cc
    widget/viewdraghelper.cc
    widget/nestedscrollinghelper.cc
    widget/cdwindow.cc
)

list(APPEND CDROID_SOURCES
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
)

list(APPEND CDROID_SOURCES
    widget/adapter.cc
    widget/headerviewlistadapter.cc
    widget/adapterview.cc
    widget/filterable.cc
    widget/abslistview.cc
    widget/recyclebin.cc
    widget/listview.cc
    widget/gridview.cc
)
if(ENABLE_SPINNER)
    list(APPEND CDROID_SOURCES
        widget/absspinner.cc
        widget/spinner.cc #spinner need AlertDialog
        widget/forwardinglistener.cc
        widget/autoscrollhelper.cc
        widget/listpopupwindow.cc
        widget/dropdownlistview.cc
    )
endif(ENABLE_SPINNER)

list(APPEND CDROID_SOURCES
    widget/absolutelayout.cc
    widget/linearlayout.cc
    widget/framelayout.cc
    widget/relativelayout.cc
    #widget/tabwidget.cc
    #widget/gridlayout.cc
    #widget/tablerow.cc
    #widget/tablayout.cc
    #widget/tablelayout.cc
)

list(APPEND CDROID_SOURCES__
    widget/activitymanager.cc
    widget/backendcairo.cc
    widget/calendarview.cc
)

list(APPEND CDROID_SOURCES  widget/viewpager.cc)

if(ENABLE_DAYTIME_WIDGETS)
    list(APPEND CDROID_SOURCES
        widget/yearpickerview.cc
        widget/daypickerpageradapter.cc
        widget/daypickerview.cc
        widget/daypickerviewpager.cc
        widget/simplemonthview.cc
    )
endif(ENABLE_DAYTIME_WIDGETS)

list(APPEND CDROID_SOURCES
    widget/drawerlayout.cc
    widget/mediacontroller.cc
    widget/patternlockview.cc

    widget/plotaxis.cc
    widget/plotobject.cc
    widget/plotpoint.cc
    widget/plotview.cc

    widget/actionbar.cc
    widget/toolbar.cc
    widget/toast.cc
)

list(APPEND CDROID_SOURCES
    widget/viewanimator.cc
    widget/viewflipper.cc
    widget/viewswitcher.cc
    widget/textswitcher.cc
    widget/imageswitcher.cc
    widget/adapterviewanimator.cc
)

if(ENABLE_DIALOGS OR ENABLE_SPINNER)
    list(APPEND CDROID_SOURCES
        app/alertcontroller.cc
        app/alertdialog.cc
        app/dialog.cc
        app/progressdialog.cc
    )
endif(ENABLE_DIALOGS)

if(ENABLE_RECYCLERVIEW)
    list(APPEND CDROID_SOURCES
        widgetEx/recyclerview/viewinfostore.cc
        widgetEx/recyclerview/viewboundscheck.cc
        widgetEx/recyclerview/staggeredgridlayoutmanager.cc
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
        #widgetEx/recyclerview/carousellayoutmanager.cc
        widgetEx/recyclerview/adapterhelper.cc
    )
endif(ENABLE_RECYCLERVIEW)

list(APPEND CDROID_SOURCES
    widgetEx/viewpager2.cc
    widgetEx/scrolleventadapter.cc
    widgetEx/plotview.cc
    widgetEx/fakedrag.cc
)

if(ENABLE_LOTTIE)
    list(APPEND CDROID_SOURCES widgetEx/rlottieview.cc)
endif(ENABLE_LOTTIE)

if(ENABLE_QRCODE)
    list(APPEND CDROID_SOURCES widgetEx/qrcodeview.cc)
endif(ENABLE_QRCODE)

if(ENABLE_BARCODE)
    list(APPEND CDROID_SOURCES widgetEx/barcodeview.cc)
endif(ENABLE_BARCODE)

if(ENABLE_NAVIGATION)
    list(APPEND CDROID_SOURCES
        navigation/navaction.cc
        navigation/navcontroller.cc
        navigation/navdeeplink.cc
        navigation/navdeeplinkbuilder.cc
        navigation/navdestination.cc
        navigation/navgraph.cc
        navigation/navgraphnavigator.cc
        navigation/navigation.cc
        navigation/navigator.cc
        navigation/navinflater.cc
        navigation/navoption.cc
        navigation/simplenavigatorprovider.cc
    )
endif(ENABLE_NAVIGATION)
