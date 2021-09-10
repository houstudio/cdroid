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
    if(a.empty()||b.empty()){
        set(0,0,0,0);
        return false;
    }
    //check if the 2 Rect intersect
    if( a.left + a.width <= b.left || b.left + b.width <= a.left || a.top + a.height <= b.top || b.top + b.height <= a.top ){
          // No intersection
          set(0,0,0,0);
          return false ;//Rect::emptyRect;
    }

    //calculate the coordinates of the intersection
    int i_x = a.left > b.left ? a.left : b.left;
    int i_y = a.top > b.top ? a.top : b.top;

    int thisWBorder  = a.left + a.width;
    int otherWBorder = b.left + b.width;
    int thisHBorder  = a.top + a.height;
    int otherHBorder = b.top + b.height;

    int i_w = thisWBorder > otherWBorder ? otherWBorder - i_x : thisWBorder - i_x;
    int i_h = thisHBorder > otherHBorder ? otherHBorder - i_y : thisHBorder - i_y;
    set(i_x,i_y,i_w,i_h);
    return true;
}

bool Rect::intersect(int l, int t, int w, int h) {
    if (this->left < l+w && l < this->right() && this->top < t+h && t < this->bottom()) {
        this->width =std::min(l+w,this->right())-this->left;
        this->height=std::min(t+h,this->bottom())-this->top;
        this->left=std::max(this->left,l);
        this->top=std::max(this->top,t);
        return true;
    }
    return false;
}

bool Rect::contains(const Rect&a)const{
    if(a.empty()||empty())return false;
    if((a.right()<=this->left)||(this->right()<=a.left)||(a.bottom()<=this->top)||(this->bottom()<a.top))
        return false;
    return true;
}

void Rect::Union(const Rect&b){
    const int mx=std::min(left,b.left);
    const int my=std::min(top,b.top);
    width=std::max(right(),b.right())-mx;
    height=std::max(bottom(),b.bottom())-my;
    left=mx;
    left=my;
}

void Rect::Union(int x,int y,int w,int h){
    const int mx=std::min(left,x);
    const int my=std::min(top,y);
    width=std::max(right(),x+w)-mx;
    height=std::max(bottom(),y+h)-my;
    left=mx;
    left=my;
}

}//end namespace
