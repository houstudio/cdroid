#ifndef __PLPLOT_VIEW_H__
#define __PLPLOT_VIEW_H__
#include <view/view.h>
#include <core/context.h>

#if ENABLE(PLPLOT)
class  plstream;
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
#endif/*ENABLE(PLPLOT)*/
#endif
