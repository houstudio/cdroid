#include <core/insets.h>

namespace cdroid{
const Insets Insets::NONE;

Insets::Insets(){
    set(0,0,0,0);
}

void Insets::set(int l, int t, int r, int b){
    left=l;
    top=t;
    right=r;
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

bool Insets::operator==(const Insets&o)const{
    return (left==o.left)&&(top==o.top)&&(right==o.right)&&(bottom==o.bottom);
}
 
bool Insets::operator!=(const Insets&o)const{
    return (left!=o.left)&&(top!=o.top)&&(right!=o.right)&&(bottom!=o.bottom);
}

}
