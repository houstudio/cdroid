#ifndef __PLOT_VIEW_H__
#define __PLOT_VIEW_H__
#include <view/view.h>
#if ENABLE_PLPLOT
class plstream;
namespace cdroid{

class PlotView:public View{
private:
   Cairo::RefPtr<Cairo::ImageSurface>mImage;
   Cairo::RefPtr<Cairo::Context>mImageContext;
   plstream*pls;
public:
   PlotView(int w,int h);
   void onDraw(Canvas&canvas)override;
   plstream*getStream()const;
};

}
#endif/*ENABLE_PLPLOT*/
#endif
