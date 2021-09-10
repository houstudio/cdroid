#include <canvas.h>
#include <graph_device.h>
#include <cairo.h>
#include <cdtypes.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <cairo-cdroid.h>
#include <vector>
#include <cairomm/context.h>
#include <cairomm/cdroid_surface.h>
#include <cairomm/region.h>
#include <cairomm/fontface.h>
#include <windowmanager.h>
#include <pixman.h>
#include <systemclock.h>

using namespace Cairo;

namespace cdroid{

GraphDevice*GraphDevice::mInst=nullptr;

GraphDevice&GraphDevice::GraphDevice::getInstance(){
    if(nullptr==mInst)
        mInst=new GraphDevice();
    return *mInst;
}

GraphDevice::GraphDevice(int fmt){
    mInst=this;
    compose_event=0;
    GFXInit();
    GFXGetScreenSize((UINT*)&width,(UINT*)&height);

    format=fmt<0?GPF_ARGB:fmt;
    GFXCreateSurface(&primarySurface,width,height,format,1);
    last_compose_time=SystemClock::uptimeMillis();
    LOGD("primarySurface=%p size=%dx%d",primarySurface,width,height);

    primaryContext=new Canvas(mInst,primarySurface);
}

GraphDevice::~GraphDevice(){
    //GFXDestroySurface(primarySurface);//surface will be destroied by CDroidSurface
    delete primaryContext;
    GFXDestroySurface(primarySurface);
    LOGD("%p Destroied",this);
}

void GraphDevice::getTextExtens(const std::string&txt,int fontsize,TextExtents&te){
    primaryContext->save();
    primaryContext->set_font_size(fontsize);
    primaryContext->get_text_extents(txt,te);
    primaryContext->restore();
}
void GraphDevice::getFontExtents(int fontsize,FontExtents&fe){
    primaryContext->save();
    primaryContext->set_font_size(fontsize);
    primaryContext->get_font_extents(fe);
    primaryContext->restore();
}

void GraphDevice::getScreenSize(int &w,int&h){
    w=width;
    h=height;
}

int GraphDevice::getScreenWidth(){
    return width;
}

int GraphDevice::getScreenHeight(){
    return height;
}

void GraphDevice::flip(){
    compose_event++;
}

bool GraphDevice::needCompose(){
    uint64_t tnow64;
    tnow64=SystemClock::uptimeMillis();
    return compose_event&&(tnow64-last_compose_time)>=30;
}

Canvas*GraphDevice::getPrimaryContext(){
    return primaryContext;
}

Canvas*GraphDevice::createContext(int width,int height){
    HANDLE cdsurface;
    GFXCreateSurface(&cdsurface,width,height,format,0);
    Canvas*graph_ctx=new Canvas(this,CDroidSurface::create(cdsurface));
    LOGD("ctx=%p cdsurface=%p  size=%dx%d",graph_ctx,cdsurface,width,height);     
    gSurfaces.push_back(graph_ctx);
    graph_ctx->dev=this;
    graph_ctx->mWidth=width;
    graph_ctx->mHeight=height;
    graph_ctx->set_antialias(ANTIALIAS_GRAY);//ANTIALIAS_SUBPIXEL);
    return graph_ctx;
}

Canvas*GraphDevice::createContext(const RECT&rect){
    Canvas*ctx=createContext(rect.width,rect.height);
    ctx->mLeft=rect.left;
    ctx->mTop=rect.top;
    return ctx;
}

void GraphDevice::add(Canvas*ctx){
     if(ctx)gSurfaces.push_back(ctx);
}

void GraphDevice::remove(Canvas*ctx){
    if(ctx==nullptr)return;
    const RECT rcw={ctx->mLeft,ctx->mTop,ctx->mWidth,ctx->mHeight};
    auto itw=std::find(gSurfaces.begin(),gSurfaces.end(),ctx);
    if(itw==gSurfaces.end()){
        LOGD_IF(itw==gSurfaces.end(),"context %p not found",ctx);
	return ;
    }
    for(auto itr=gSurfaces.begin();itr!=itw;itr++){
       Canvas*c=(*itr);
       RECT r={c->mLeft,c->mTop,c->mWidth,c->mHeight};
       r.intersect(rcw);
       r.offset(-c->mLeft,-c->mTop);
       c->mInvalidRgn->do_union((const RectangleInt&)r);
    }
    if(itw!=gSurfaces.end()){
        gSurfaces.erase(itw);
        flip();
    }
}

void GraphDevice::ComposeSurfaces(){
    int rects=0;
    long t2,t1=SystemClock::uptimeMillis();

    std::sort(gSurfaces.begin(),gSurfaces.end(),[](Canvas*c1,Canvas*c2){
        return c2->mLayer-c1->mLayer>0;
    });

    for(int i=0;i<primaryContext->mInvalidRgn->get_num_rectangles();i++){
        RectangleInt r=primaryContext->mInvalidRgn->get_rectangle(i);
        GFXFillRect(primarySurface,(const GFXRect*)&r,0);
    } 
    primaryContext->mInvalidRgn->do_xor(primaryContext->mInvalidRgn);

    for(auto s=gSurfaces.begin();s!=gSurfaces.end();s++){
        if( (*s)->mInvalidRgn->empty() )continue;
        rects+=(*s)->blit2Device(primarySurface);
    }
    GFXFlip(primarySurface);
    
    t2=SystemClock::uptimeMillis();
    LOGV("ComposeSurfaces %d surfaces %d rects used %d ms",gSurfaces.size(),rects,t2-t1);
    last_compose_time=SystemClock::uptimeMillis();
    compose_event=0;
}
}//end namespace
