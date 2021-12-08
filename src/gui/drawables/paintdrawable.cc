#include <drawables/paintdrawable.h>
namespace cdroid{

PaintDrawable::PaintDrawable(){
}

PaintDrawable::PaintDrawable(int color){
}

void PaintDrawable::setCornerRadius(float radius){
    std::vector<int>radii;
    if(radius>0)
        for(int i=0;i<8;i++)radii.push_back((int)radius);
    setCornerRadii(radii);
}

void PaintDrawable::setCornerRadii(const std::vector<int>& radii){
    if(radii.size()==0){
        if(getShape())setShape(nullptr);
    }else{
        Rect inset={0,0,0,0};
        setShape(new RoundRectShape(radii,inset,std::vector<int>()));
    }
}

}
