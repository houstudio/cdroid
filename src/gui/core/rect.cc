#include <rect.h>
#include <algorithm>

namespace cdroid{
void Point::set(int x_,int y_){
    x=x_;
    y=y_;
}

void Rect::set(int x_,int y_,int w,int h){
   x=x_;y=y_;width=w;height=h;
}

bool Rect::empty()const{
    return width<=0||height<=0;
}

void Rect::setEmpty(){
    set(0,0,0,0);
}

bool Rect::operator==(const Rect&b)const{
    return (x==b.x)&&(y==b.y)&&(width==b.width)&&(height==b.height);
}

bool Rect::operator!=(const Rect&b)const{
    return (x!=b.x)&&(y!=b.y)&&(width!=b.width)&&(height!=b.height);
}

void Rect::offset(int dx,int dy){
    x+=dx;
    y+=dy;
}

void Rect::inflate(int dx,int dy){
   x-=dx;y-=dy;
   width+=(dx+dx);
   height+=(dy+dy);
}

bool Rect::intersect(const Rect&b){
    return intersect(*this,b);
}

bool Rect::contains(int xx,int yy)const{
    return xx>=x&&xx<right() && yy>=y&&yy<bottom();
}

bool Rect::intersect(const Rect&a,const Rect&b){
    if(a.empty()||b.empty()){
        set(0,0,0,0);
        return false;
    }
    //check if the 2 Rect intersect
    if( a.x + a.width <= b.x || b.x + b.width <= a.x || a.y + a.height <= b.y || b.y + b.height <= a.y ){
          // No intersection
          set(0,0,0,0);
          return false ;//Rect::emptyRect;
    }

    //calculate the coordinates of the intersection
    int i_x = a.x > b.x ? a.x : b.x;
    int i_y = a.y > b.y ? a.y : b.y;

    int thisWBorder  = a.x + a.width;
    int otherWBorder = b.x + b.width;
    int thisHBorder  = a.y + a.height;
    int otherHBorder = b.y + b.height;

    int i_w = thisWBorder > otherWBorder ? otherWBorder - i_x : thisWBorder - i_x;
    int i_h = thisHBorder > otherHBorder ? otherHBorder - i_y : thisHBorder - i_y;
    set(i_x,i_y,i_w,i_h);
    return true;
}

bool Rect::intersect(int l, int t, int w, int h) {
    if (this->x < l+w && l < this->right() && this->y < t+h && t < this->bottom()) {
        this->width =std::min(l+w,this->right())-this->x;
        this->height=std::min(t+h,this->bottom())-this->y;
        this->x=std::max(this->x,l);
        this->y=std::max(this->y,t);
        return true;
    }
    return false;
}

bool Rect::contains(const Rect&a)const{
    if(a.empty()||empty())return false;
    if((a.right()<=this->x)||(this->right()<=a.x)||(a.bottom()<=this->y)||(this->bottom()<a.y))
        return false;
    return true;
}

void Rect::Union(const Rect&b){
    const int mx=std::min(x,b.x);
    const int my=std::min(y,b.y);
    width=std::max(right(),b.right())-mx;
    height=std::max(bottom(),b.bottom())-my;
    x=mx;
    y=my;
}

}//end namespace
