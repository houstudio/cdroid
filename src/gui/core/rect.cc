#include <rect.h>
#include <algorithm>

namespace cdroid{
void Point::set(int x_,int y_){
    x=x_;
    y=y_;
}

void Rect::set(int x_,int y_,int w,int h){
   left = x_;
   top=y_;
   width=w;
   height=h;
}

bool Rect::empty()const{
    return width<=0||height<=0;
}

void Rect::setEmpty(){
    set(0,0,0,0);
}

bool Rect::operator==(const Rect&b)const{
    return (left==b.left)&&(top==b.top)&&(width==b.width)&&(height==b.height);
}

bool Rect::operator!=(const Rect&b)const{
    return (left!=b.left)&&(top!=b.top)&&(width!=b.width)&&(height!=b.height);
}

void Rect::offset(int dx,int dy){
    left+=dx;
    top+=dy;
}

void Rect::inflate(int dx,int dy){
   left-=dx;top-=dy;
   width+=(dx+dx);
   height+=(dy+dy);
}

bool Rect::intersect(const Rect&b){
    return intersect(*this,b);
}

bool Rect::contains(int xx,int yy)const{
    return xx>=left&&xx<right() && yy>=top&&yy<bottom();
}

bool Rect::intersect(const Rect&a,const Rect&b){
    int x1, y1, x2, y2;

    x1 = std::max (a.left, b.left);
    y1 = std::max (a.top , b.top);
    /* Beware the unsigned promotion, fortunately we have bits to spare
     * as (CAIRO_RECT_INT_MAX - CAIRO_RECT_INT_MIN) < UINT_MAX
     */
    x2 = std::min (a.left + (int) a.width, b.left + (int) b.width);
    y2 = std::min (a.top + (int) a.height, b.top + (int) b.height);

    if (x1 >= x2 || y1 >= y2) {
        left = 0;
        top = 0;
        width  = 0;
        height = 0;

        return false;
    } else {
        left = x1;
        top = y1;
        width  = x2 - x1;
        height = y2 - y1;
        return true;
    }
    return true;
}

bool Rect::intersect(int l, int t, int w, int h) {
    return intersect(*this,Make(l,t,w,h));
}

bool Rect::contains(const Rect&a)const{
    if(a.empty()||empty())return false;
    if((a.right()<=this->left)||(this->right()<=a.left)||(a.bottom()<=this->top)||(this->bottom()<a.top))
        return false;
    return true;
}

void Rect::Union(const Rect&b){
    int x1, y1, x2, y2;
    x1 = std::min (left, b.left);
    y1 = std::min (top, b.top);
    /* Beware the unsigned promotion, fortunately we have bits to spare
     * as (CAIRO_RECT_INT_MAX - CAIRO_RECT_INT_MIN) < UINT_MAX
     */
    x2 = std::max (b.left + (int) b.width,  left + width);
    y2 = std::max (b.top + (int) b.height, top + height);

    left = x1;
    top = y1;
    width  = x2 - x1;
    height = y2 - y1;
}

void Rect::Union(int x,int y,int w,int h){
    Union(Make(x,y,w,h));
}

}//end namespace
