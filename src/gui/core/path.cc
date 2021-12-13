#include <core/path.h>
#include <cairomm/surface.h>
#include <cairomm/context.h>
namespace cdroid{

Path::Path(){
    mCTX=Cairo::Context::create(Cairo::ImageSurface::create(Cairo::Surface::Format::A8,1,1));
}

Path::Path(const Path&o):Path(){
    o.append_to_context(mCTX);
}

void Path::reset(){
    mCTX->begin_new_path();
}

void Path::begin_new_sub_path(){
    mCTX->begin_new_sub_path();
}

void Path::close_path(){
    mCTX->close_path();
}

void Path::append_to_context(Cairo::Context*to)const{
    const Cairo::RefPtr<Cairo::Path>from=Cairo::make_refptr_for_instance<Cairo::Path>(mCTX->copy_path());
    to->append_path(*from);
}

void Path::append_to_context(const Cairo::RefPtr<Cairo::Context>&to)const{
    Cairo::RefPtr<Cairo::Path>from=Cairo::make_refptr_for_instance<Cairo::Path>(mCTX->copy_path()); 
    to->append_path(*from);
}

void Path::set_fill_rule(Cairo::Context::FillRule fill_rule){
    mCTX->set_fill_rule(fill_rule);
}

void Path::move_to(double x,double y){
    mCTX->move_to(x,y);
}

void Path::line_to(double x,double y){
    mCTX->line_to(x,y);
}

void Path::curve_to(double x1, double y1, double x2, double y2, double x3, double y3){
    mCTX->curve_to(x1,y1,x2,y2,x3,y3);
}

void Path::arc(double xc, double yc, double radius, double angle1, double angle2){
    mCTX->arc(xc,yc,radius,angle1,angle2);
}

void Path::rectangle(double x, double y, double width, double height){
    mCTX->rectangle(x,y,width,height);
}

void Path::round_rectangle(double x,double y,double width,double height,const std::vector<float>& radii){
    //const RectF rect={x,y,width,height};
    round_rectangle({float(x),float(y),float(width),float(height)},radii);
}
void Path::round_rectangle(const RectF&rect,const std::vector<float>& radii){
    constexpr double circleControlPoint=0.447715;
    move_to(rect.left+radii[0],rect.top);
    line_to(rect.right()-radii[2],rect.top);
    if((radii[2]>0||radii[3])){//topright
         curve_to(rect.right()-radii[2]*circleControlPoint,rect.top,
             rect.right(),rect.top+radii[3]*circleControlPoint,
             rect.right(),rect.top+radii[3]); 
    }
    line_to(rect.right(),rect.bottom()-radii[5]);

    if(radii[4]>0||radii[5]){//bottomright 
         curve_to(rect.right(),rect.bottom()-radii[5]*circleControlPoint,
             rect.right()-radii[4]*circleControlPoint, rect.bottom(),
             rect.right()-radii[4],rect.bottom());
    }
    line_to(rect.left+radii[4],rect.bottom());

    if(radii[6]>0||radii[7]>0){//bottomleft
        curve_to(rect.left+radii[6]*circleControlPoint,rect.bottom(),
            rect.left,rect.bottom()-radii[7]*circleControlPoint,
            rect.left,rect.bottom()-radii[7]);
    }
    line_to(rect.left,rect.top+radii[7]);

    if(radii[0]>0||radii[1]>0){//topleft
        curve_to(rect.left,rect.top+radii[1]*circleControlPoint,
           rect.left+radii[0]*circleControlPoint,rect.top,
           rect.left+radii[0],rect.top);
    }
    mCTX->close_path();//closeSubpath();
}

}
