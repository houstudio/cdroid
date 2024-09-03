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
    #view/menu.cc
    )

list(APPEND CDROID_SOURCES
    view/gesturedetector.cc
    view/scalegesturedetector.cc
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
    widget/adapter.cc
    widget/headerviewlistadapter.cc
    widget/adapterview.cc
    widget/filterable.cc
    widget/abslistview.cc
    widget/recyclebin.cc
    widget/listview.cc
    widget/gridview.cc
    widget/yearpickerview.cc
    )

list(APPEND CDROID_SOURCES__
    widget/absspinner.cc
    widget/spinner.cc
    widget/forwardinglistener.cc
    widget/listpopupwindow.cc
    widget/dropdownlistview.cc
)

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

list(APPEND CDROID_SOURCES
    widget/nestedscrollview.cc
    widget/radiogroup.cc
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
    widget/switch.cc
    widget/analogclock.cc
    widget/numberpicker.cc
    widget/popupwindow.cc
    widget/candidateview.cc
    widget/keyboardview.cc
)

list(APPEND CDROID_SOURCES__
    widget/actionbar.cc
    widget/activitymanager.cc
    widget/autoscrollhelper.cc
    widget/backendcairo.cc
    widget/calendarview.cc
)
list(APPEND CDROID_SOURCES  widget/viewpager.cc)

list(APPEND CDROID_SOURCES_
    widget/daypickerpageradapter.cc
    widget/daypickerview.cc
    widget/daypickerviewpager.cc
    widget/simplemonthview.cc

    widget/drawerlayout.cc
    widget/mediacontroller.cc
    widget/patternlockview.cc

    widget/plotaxis.cc
    widget/plotobject.cc
    widget/plotpoint.cc
    widget/plotview.cc

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

list(APPEND CDROID_SOURCES_
    app/alertcontroller.cc
    app/alertdialog.cc
    app/dialog.cc
    app/progressdialog.cc
    )
