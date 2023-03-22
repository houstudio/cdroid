#include <optionpicker.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <widget/R.h>
#include <R.h>

DECLARE_WIDGET(OptionPicker)
OptionPicker::OptionPicker(int w,int h):RelativeLayout(w,h){

}

OptionPicker::OptionPicker(Context*ctx,const AttributeSet&attr):RelativeLayout(ctx,attr){
    LayoutInflater::from(ctx)->inflate("@layout/optionitem",this);
    mNumberPicker = (NumberPicker*)findViewById(w9::R::id::numpicker);
    mNumberPicker->setVisibility(View::GONE);
    mNumberPicker->setMinValue(0);
    mNumberPicker->setMaxValue(5);
    mText1=(TextView*)findViewById(w9::R::id::text1);
    mText2=(TextView*)findViewById(w9::R::id::text2);
    mText1->setText(attr.getString("text1"));
    mText2->setText(attr.getString("text2"));
    TextView*v=dynamic_cast<EditText*>(findViewById(cdroid::R::id::numberpicker_input));
    if(v){
	 v->setVisibility(View::GONE);
	 v->setTextSize(48);
    }
    mNumberPicker->setTextSize(40);
    LOGD("mNumberPicker=%p text=%p/%p",mNumberPicker,mText1,mText2);
    mNumberPicker->setOnValueChangedListener([this](NumberPicker&v,int ov,int nv){
        std::string txt=std::to_string(nv);
	std::vector<std::string>displayNames=v.getDisplayedValues();
	if(ov<mValues.size()&&ov>=0)  ov=mValues.at(ov);
	if(nv<mValues.size()&&nv>=0)  nv=mValues.at(nv);
	if(nv<displayNames.size()&&nv>=0)  txt=v.getDisplayedValues().at(nv);
        mText1->setText(txt);
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
    const int count=std::max(values.size(),names.size());
    mValues=values;
    mNumberPicker->setMinValue(0);
    mNumberPicker->setMaxValue(count-1);
    mNumberPicker->setDisplayedValues(names);
}

void OptionPicker::showOptions(bool on){
    mNumberPicker->setVisibility(on?View::VISIBLE:View::GONE);
    LOGV("%p:%d visible=%d",this,mID,on);
}
