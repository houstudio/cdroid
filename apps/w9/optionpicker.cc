#include <optionpicker.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <R.h>

DECLARE_WIDGET(OptionPicker)
OptionPicker::OptionPicker(int w,int h):RelativeLayout(w,h){

}

OptionPicker::OptionPicker(Context*ctx,const AttributeSet&attr):RelativeLayout(ctx,attr){
    LayoutInflater::from(ctx)->inflate("@layout/optionitem",this);
    mNumberPicker = (NumberPicker*)findViewById(w9::R::id::numpicker);
    mNumberPicker->setVisibility(View::GONE);
    mText1=(TextView*)findViewById(w9::R::id::text1);
    mText2=(TextView*)findViewById(w9::R::id::text2);
    mText1->setText(attr.getString("text1"));
    mText2->setText(attr.getString("text2"));
    LOGD("mNumberPicker=%p text=%p/%p",mNumberPicker,mText1,mText2);
    mNumberPicker->setOnValueChangedListener([this](NumberPicker&v,int ov,int nv){
        mText1->setText(std::to_string(nv));
	if(ov<mValues.size()&&ov>=0)  ov=mValues.at(ov);
	if(nv<mValues.size()&&nv>=0)  nv=mValues.at(nv);
        if(mOnValueChangedListener)
	    mOnValueChangedListener(v,ov,nv);
    });
}

NumberPicker&OptionPicker::getPicker(){
    return *mNumberPicker;
}

void OptionPicker::setText(const std::string&text){
    mText1->setText(text);
}

void OptionPicker::setText(const std::string&text1,const std::string&text2){
    mText1->setText(text1);
    mText2->setText(text2);
}

void OptionPicker::setOnValueChangedListener(NumberPicker::OnValueChangeListener onValueChangedListener){
    mOnValueChangedListener=onValueChangedListener;
}

void OptionPicker::setValuedName(const std::vector<int>&values,const std::vector<std::string>&names){
    mValues=values;
    mNumberPicker->setMinValue(0);
    mNumberPicker->setMaxValue(values.size());
    mNumberPicker->setDisplayedValues(names);
}

void OptionPicker::showOptions(bool on){
    mNumberPicker->setVisibility(on?View::VISIBLE:View::GONE);
    LOGV("%p:%d visible=%d",this,mID,on);
}

static int makeMeasureSpec(int measureSpec, int maxSize){
    if (maxSize == -1/*SIZE_UNSPECIFIED*/) {
        return measureSpec;
    }
    int size = MeasureSpec::getSize(measureSpec);
    int mode = MeasureSpec::getMode(measureSpec);
    switch (mode) {
    case MeasureSpec::EXACTLY:     return measureSpec;
    case MeasureSpec::AT_MOST:     return MeasureSpec::makeMeasureSpec(std::min(size, maxSize), MeasureSpec::EXACTLY);
    case MeasureSpec::UNSPECIFIED: return MeasureSpec::makeMeasureSpec(maxSize, MeasureSpec::EXACTLY);
    default:        throw std::string("Unknown measure mode: ")+std::to_string(mode);
    }
}

int OptionPicker::resolveSizeAndStateRespectingMinSize(int minSize, int measuredSize, int measureSpec) {
    if (minSize != -1/*SIZE_UNSPECIFIED*/) {
        int desiredWidth = std::max(minSize, measuredSize);
        return resolveSizeAndState(desiredWidth, measureSpec, 0);
    } else {
        return measuredSize;
    }
}

void OptionPicker::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    RelativeLayout::onMeasure(widthMeasureSpec,heightMeasureSpec);
    return;
    // Try greedily to fit the max width and height.
    const int mMaxWidth=-1, mMaxHeight=-1;
    const int mMinWidth=-1, mMinHeight=-1;
    int newWidthMeasureSpec = makeMeasureSpec(widthMeasureSpec, mMaxWidth);
    int newHeightMeasureSpec = makeMeasureSpec(heightMeasureSpec, mMaxHeight);
    RelativeLayout::onMeasure(newWidthMeasureSpec, newHeightMeasureSpec);
    // Flag if we are measured with width or height less than the respective min.
    int widthSize = resolveSizeAndStateRespectingMinSize(mMinWidth, getMeasuredWidth(),
                widthMeasureSpec);
    int heightSize = resolveSizeAndStateRespectingMinSize(mMinHeight, getMeasuredHeight(),
                heightMeasureSpec);
    LOGD("%p:%d setMeasuredDimension(%x/%d,%x/%d)",this,mID,widthSize,widthSize,heightSize,heightSize);
    setMeasuredDimension(widthSize, heightSize);
}

void OptionPicker::onLayout(bool changed, int left, int top, int width, int height){
    RelativeLayout::onLayout(changed,left,top,width,height);
    return ;
    const int inptTxtMsrdWdth = mText2->getMeasuredWidth();
    const int inptTxtMsrdHght = mText2->getMeasuredHeight();
    const int msrdWdth = getMeasuredWidth();
    const int msrdHght = getMeasuredHeight();
    mText2->layout(0, msrdHght-inptTxtMsrdHght, inptTxtMsrdWdth, inptTxtMsrdHght);
    mText1->layout(0,msrdHght-inptTxtMsrdHght*2,inptTxtMsrdWdth, inptTxtMsrdHght);
    LOGD("%p:%d textSize=(%d,%d) measuredsize=%dx%d",this,mID,inptTxtMsrdWdth,inptTxtMsrdHght,msrdWdth,msrdHght);
    mNumberPicker->layout(0,0,inptTxtMsrdWdth,msrdHght-inptTxtMsrdHght*2);
}
