#ifndef __CAIRO_WRAPPER_H__
#define __CAIRO_WRAPPER_H__
#include<cairo.h>

class CairoPath{

};
class CairoContext{
protected:
   struct _cairo *cr;
public:

   void arc(float xc,float yc,float radius,float angle1,float angle2){cairo_arc(cr,xc,yc,radius,angle1,angle2);}

   void moveTo(float x,float y){cairo_move_to(cr,x,y);}

   void lineTo(float x,float y){cairo_line_to(cr,x,y);}    

   void curveTo(float x1,float y1,float x2,float y2,float x3,float y3){
        cairo_curve_to(cr,x1,y1,x2,y2,x3,y3);
   }

   void rectangle(float x,float y,float w,float h){cairo_rectangle(cr,x,y,w,h);}

   void closePath(){cairo_close_path(cr);}

   void clip(){
        cairo_clip(cr);
   }
   void resetClip(){
        cairo_reset_clip(cr);
   }

   void setColor(unsigned char r,unsigned char g,unsigned char b,unsigned char a=255){
       cairo_set_source_rgba(cr,r/255.f,g/255.f,b/255.f,a/255.f);
   }
   void setColor(float r,float g ,float b,float a=1.f){
       cairo_set_source_rgba(cr,r,g,b,a);
   }
   void stroke(bool clear_path=true){
       if(clear_path)cairo_stroke(cr);
       else cairo_stroke_preserve(cr);
   }

   void fill(bool clear_path=true){
       if(clear_path)cairo_fill(cr);
       else cairo_fill_preserve(cr);
   }
   
   void paint(){
       cairo_paint(cr);
   }
   void showText(const char*utf8){
       cairo_show_text(cr,utf8);
   }
   void textPath(const char*utf8){
       cairo_text_path(cr,utf8);
   }
   
};

#endif

