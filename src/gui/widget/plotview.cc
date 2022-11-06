#include <widget/plotview.h>
#include <gui_features.h>
#ifdef ENABLE_PLPLOT
#include <plstream.h>
#endif
namespace cdroid{

PlotView::PlotView(int w,int h):View(w,h){
    pls=nullptr;
#ifdef ENABLE_PLPLOT
    pls=new plstream();
    //int argc=0;
    //char*argv[]={nullptr};
    //pls->parseopts(&argc,argv,PL_PARSE_FULL|PL_PARSE_NOPROGRAM);
    pls->sdev("extcairo");
    mImage=ImageSurface::create(Surface::Format::ARGB32,w,h);
    mImageContext=Cairo::Context::create(mImage);
    pls->init();
    pls->cmd(PLESC_DEVINIT,mImageContext->cobj());
#endif
}

void PlotView::onDraw(Canvas&canvas){
#ifdef ENABLE_PLPLOT
    canvas.save();
    canvas.set_operator(Cairo::Context::Operator::OVER);
    canvas.set_source(mImage,.0f,.0f);
    canvas.rectangle(0,0,getWidth(),getHeight());
    canvas.fill();
    canvas.restore();
#else
    View::onDraw(canvas);
#endif
}

plstream*PlotView::getStream()const{
    return pls;
}

}

