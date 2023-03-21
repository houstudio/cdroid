#include <washoptions.h>
#include <cdroid.h>
#include <R.h>
#include <optionpicker.h>

WashOptionsWindow::WashOptionsWindow(int options):Window(0,0,-1,-1){
    LayoutInflater::from(getContext())->inflate("@layout/options",this);
    ViewGroup*vg=dynamic_cast<ViewGroup*>(getChildAt(0));
    for(int i=0;i<vg->getChildCount();i++){
	vg->getChildAt(i)->setOnClickListener(std::bind(&WashOptionsWindow::onOptionClick,this,std::placeholders::_1));
    }
}

void WashOptionsWindow::onOptionClick(View&v){
    ViewGroup*vg=v.getParent();
    std::vector<int>values;
    std::vector<std::string>array;
    getContext()->getArray("w9:array/cleverwash",array);
    LOGD("array.size=%d",array.size());
    dynamic_cast<OptionPicker&>(v).setValuedName(values,array);
    for(int i=0;i<vg->getChildCount();i++){
        OptionPicker*op=dynamic_cast<OptionPicker*>(vg->getChildAt(i));
        op->showOptions(op==&v);
    }
    invalidate();
}
