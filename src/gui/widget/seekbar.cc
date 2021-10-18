#include <widget/seekbar.h>
namespace cdroid{

SeekBar::SeekBar(Context*ctx,const AttributeSet& attrs,const std::string&defstyle)
  :AbsSeekBar(ctx,attrs,defstyle){
}

SeekBar::SeekBar(int w,int h):AbsSeekBar(w,h){
}

void SeekBar::onProgressRefresh(float scale, bool fromUser, int progress){
    AbsSeekBar::onProgressRefresh(scale, fromUser, progress);
    if (mOnSeekBarChangeListener != nullptr) {
         mOnSeekBarChangeListener(*this, progress, fromUser);
    }
}

}
