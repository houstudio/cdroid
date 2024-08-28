#include <widget/seekbar.h>
namespace cdroid{

DECLARE_WIDGET2(SeekBar,"cdroid:attr/seekBarStyle")

SeekBar::SeekBar(Context*ctx,const AttributeSet& attrs)
  :AbsSeekBar(ctx,attrs){
}

SeekBar::SeekBar(int w,int h):AbsSeekBar(w,h){
}

void SeekBar::onProgressRefresh(float scale, bool fromUser, int progress){
    AbsSeekBar::onProgressRefresh(scale, fromUser, progress);
    if (mOnSeekBarChangeListener.onProgressChanged) {
         mOnSeekBarChangeListener.onProgressChanged(*this, progress, fromUser);
    }
}

void SeekBar::onStartTrackingTouch() {
    AbsSeekBar::onStartTrackingTouch();
    if (mOnSeekBarChangeListener.onStartTrackingTouch) {
        mOnSeekBarChangeListener.onStartTrackingTouch(*this);
    }
}

void SeekBar::onStopTrackingTouch() {
    AbsSeekBar::onStopTrackingTouch();
    if (mOnSeekBarChangeListener.onStopTrackingTouch) {
        mOnSeekBarChangeListener.onStopTrackingTouch(*this);
    }
}

void SeekBar::setOnSeekBarChangeListener(const OnSeekBarChangeListener& l){
     mOnSeekBarChangeListener = l;
}

}
