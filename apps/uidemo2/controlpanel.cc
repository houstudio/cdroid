#include <controlpanel.h>
#include <view/layoutinflater.h>
#include <R.h>
namespace cdroid{

ControlPanel::ControlPanel(Context*ctx,const AttributeSet&atts)
  :FrameLayout(ctx,atts){
}

void ControlPanel::loadFrame(const std::string&resid){
     LayoutInflater::from(mContext)->inflate(resid,this,true);
}
void ControlPanel::init(){
}
//////////////////////////////////////////////////////////////////////////////

SencePanel::SencePanel(Context*ctx,const AttributeSet&atts)
   :ControlPanel(ctx,atts){
}

void SencePanel::init(){
    mGridView= (GridView*)findViewById(uidemo2::R::id::gridview);
}

SencePanel::~SencePanel(){
}

}

