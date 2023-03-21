#pragma once
#include <widget/relativelayout.h>
#include <widget/numberpicker.h>
#include <widget/textview.h>

class OptionPicker:public RelativeLayout{
protected:
    NumberPicker*mNumberPicker;
    TextView* mText1,*mText2;
    std::vector<int>mValues;
    NumberPicker::OnValueChangeListener mOnValueChangedListener;
public:
    OptionPicker(int,int);
    OptionPicker(Context*,const AttributeSet&attr);
    NumberPicker&getPicker();
    void setText(const std::string&text);
    void setText(const std::string&,const std::string&);
    void setOnValueChangedListener(NumberPicker::OnValueChangeListener onValueChangedListener);
    /*values used for uart commandid,names used for display,values.size==names.size*/
    void setValuedName(const std::vector<int>&values,const std::vector<std::string>&names);
    void showOptions(bool on);
};

