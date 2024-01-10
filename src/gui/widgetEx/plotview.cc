#if ENABLE_PLPLOT
#include <widgetEx/plotview.h>
#include <plstream.h>
namespace cdroid{

PlotView::PlotView(int w,int h):View(w,h){
    pls=new plstream();
    //int argc=0;
    //char*argv[]={nullptr};
    //pls->parseopts(&argc,argv,PL_PARSE_FULL|PL_PARSE_NOPROGRAM);
    pls->sdev("extcairo");
    mImage=ImageSurface::create(Surface::Format::ARGB32,w,h);
    mImageContext=Cairo::Context::create(mImage);
    pls->init();
    pls->cmd(PLESC_DEVINIT,mImageContext->cobj());
}

void PlotView::onDraw(Canvas&canvas){
    canvas.save();
    canvas.set_operator(Cairo::Context::Operator::OVER);
    canvas.set_source(mImage,.0f,.0f);
    canvas.rectangle(0,0,getWidth(),getHeight());
    canvas.fill();
    canvas.restore();
}

plstream*PlotView::getStream()const{
    return pls;
}

}
#endif/*ENABLE_PLPLOT*/
