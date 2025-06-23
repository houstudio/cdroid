#include <core/insets.h>

namespace cdroid{
const Insets Insets::NONE;

Insets::Insets(){
    set(0,0,0,0);
}

void Insets::set(int l, int t, int r, int b){
    left = l;
    top  = t;
    right= r;
    bottom=b;
}

Insets::Insets(int l, int t, int r, int b){
    set(l,t,r,b);
}

Insets Insets::of(int left, int top, int right, int bottom){
    Insets insets(left,top,right,bottom);
    return insets;
}

Insets Insets::of(const Rect& r){
    Insets insets(r.left,r.top,r.width,r.height);
    return insets;
}

const Rect Insets::toRect()const{
    return {left,top,right,bottom};
}

Insets Insets::add(const Insets&a,const Insets&b){
    return Insets::of(a.left + b.left, a.top + b.top, a.right + b.right, a.bottom + b.bottom);
}

Insets Insets::subtract(const Insets&a,const Insets&b){
    return Insets::of(a.left - b.left, a.top - b.top, a.right - b.right, a.bottom - b.bottom);
}

Insets Insets::max(const Insets&a,const Insets&b){
    return Insets::of(std::max(a.left, b.left), std::max(a.top, b.top),
                std::max(a.right, b.right), std::max(a.bottom, b.bottom));
}

Insets Insets::min(const Insets&a,const Insets&b){
    return Insets::of(std::min(a.left, b.left), std::min(a.top, b.top),
                std::min(a.right, b.right), std::min(a.bottom, b.bottom));
}

bool Insets::operator==(const Insets&o)const{
    return (left==o.left)&&(top==o.top)&&(right==o.right)&&(bottom==o.bottom);
}
 
bool Insets::operator!=(const Insets&o)const{
    return (left!=o.left)&&(top!=o.top)&&(right!=o.right)&&(bottom!=o.bottom);
}

}
