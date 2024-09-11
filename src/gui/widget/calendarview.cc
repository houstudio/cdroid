#include <widget/calendarview.h>
#include <widget/calendarviewmaterialdelegate.h>
namespace cdroid{

CalendarView::CalendarView(int w,int h):FrameLayout(w,h){
}

CalendarView::CalendarView(Context*ctx,const AttributeSet&atts)
  :FrameLayout(ctx,atts){
}


}//namespace
