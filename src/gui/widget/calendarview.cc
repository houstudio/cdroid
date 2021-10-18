#include <widget/calendarview.h>
namespace cdroid{

CalendarView::CalendarView(int w,int h):FrameLayout(w,h){
}

CalendarView::CalendarView(Context*ctx,const AttributeSet&atts,const std::string&defstyle)
  :FrameLayout(ctx,atts,defstyle){
}


}//namespace
