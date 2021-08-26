#include <rect.h>
#include <algorithm>

namespace cdroid{
void Point::set(int x_,int y_){
    x=x_;
    y=y_;
}

void Rectangle::set(int x_,int y_,int w,int h){
   x=x_;y=y_;width=w;height=h;
}

bool Rectangle::empty()const{
    return width<=0||height<=0;
}

void Rectangle::setEmpty(){
    set(0,0,0,0);
}

bool Rectangle::operator==(const Rectangle&b)const{
    return (x==b.x)&&(y==b.y)&&(width==b.width)&&(height==b.height);
}

bool Rectangle::operator!=(const Rectangle&b)const{
    return (x!=b.x)&&(y!=b.y)&&(width!=b.width)&&(height!=b.height);
}

void Rectangle::offset(int dx,int dy){
    x+=dx;
    y+=dy;
}

void Rectangle::inflate(int dx,int dy){
   x-=dx;y-=dy;
   width+=(dx+dx);
   height+=(dy+dy);
}

bool Rectangle::intersect(const Rectangle&b){
    return intersect(*this,b);
}

bool Rectangle::contains(int xx,int yy)const{
    return xx>=x&&xx<right() && yy>=y&&yy<bottom();
}

bool Rectangle::intersect(const Rectangle&a,const Rectangle&b){
    if(a.empty()||b.empty()){
        set(0,0,0,0);
        return false;
    }
    //check if the 2 Rectangle intersect
    if( a.x + a.width <= b.x || b.x + b.width <= a.x || a.y + a.height <= b.y || b.y + b.height <= a.y ){
          // No intersection
          set(0,0,0,0);
          return false ;//Rectangle::emptyRectangle;
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

bool Rectangle::intersect(int l, int t, int w, int h) {
    if (this->x < l+w && l < this->right() && this->y < t+h && t < this->bottom()) {
        this->width =std::min(l+w,this->right())-this->x;
        this->height=std::min(t+h,this->bottom())-this->y;
        this->x=std::max(this->x,l);
        this->y=std::max(this->y,t);
        return true;
    }
    return false;
}

bool Rectangle::contains(const Rectangle&a)const{
    if(a.empty()||empty())return false;
    if((a.right()<=this->x)||(this->right()<=a.x)||(a.bottom()<=this->y)||(this->bottom()<a.y))
        return false;
    return true;
}

}//end namespace
