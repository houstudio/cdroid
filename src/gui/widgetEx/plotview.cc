#include <widgetEx/plotview.h>
#if ENABLE(PLPLOT)
#include <plstream.h>
namespace cdroid{

DECLARE_WIDGET(PLPlotView)

PLPlotView::PLPlotView(int w,int h):View(w,h){
    initView();
}

PLPlotView(Context*ctx,const AttributeSet&attrs):View(ctx,attrs){
    pls = new plstream();
    pls->sdev("extcairo");
    pls->init();
}

PLPlotView::~PLPlotView(){
    delete pls;
}

void PLPlotView::onSizeChanged(int w,int h,int ow,int oh){
    mImage = ImageSurface::create(Surface::Format::ARGB32,w,h);
    mImageContext = Cairo::Context::create(mImage);
    pls->cmd(PLESC_DEVINIT,mImageContext->cobj());
}

void PLPlotView::onDraw(Canvas&canvas){
    canvas.save();
    canvas.set_operator(Cairo::Context::Operator::OVER);
    canvas.set_source(mImage,0.f,0.f);
    canvas.rectangle(0,0,getWidth(),getHeight());
    canvas.fill();
    canvas.restore();
}

plstream*PLPlotView::getStream()const{
    return pls;
}

}
#endif/*ENABLE(PLPLOT)*/
