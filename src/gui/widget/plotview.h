#ifndef __PLOT_VIEW_H__
#define __PLOT_VIEW_H__
#include <widget/view.h>
class plstream;
namespace cdroid{

class PlotView:public View{
private:
   RefPtr<ImageSurface>mImage;
   RefPtr<Cairo::Context>mImageContext;
   plstream*pls;
public:
   PlotView(int w,int h);
   void onDraw(Canvas&canvas)override;
   plstream*getStream()const;
};

}

#endif
