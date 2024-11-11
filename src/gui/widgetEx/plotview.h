#ifndef __PLPLOT_VIEW_H__
#define __PLPLOT_VIEW_H__
#include <view/view.h>
#include <core/context.h>

#if ENABLE(PLPLOT)
class  plstream;
namespace cdroid{

class PLPlotView:public View{
private:
   Cairo::RefPtr<Cairo::ImageSurface>mImage;
   Cairo::RefPtr<Cairo::Context>mImageContext;
   plstream*pls;
private:
   void initView();
protected:
   void onSizeChanged(int w,int h,int ow,int oh)override;
public:
   PLPlotView(int w,int h);
   PLPlotView(Context*,const AttributeSet&attrs);
   ~PLPlotView();
   void onDraw(Canvas&canvas)override;
   plstream*getStream()const;
};

}
#endif/*ENABLE(PLPLOT)*/
#endif
