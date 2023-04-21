#include <washoptions.h>
#include <cdroid.h>
#include <R.h>
#include <optionpicker.h>
#include <cdlog.h>

WashOptionsWindow::WashOptionsWindow(int x,int y,int w,int h):Window(x,y,w,h){
    const char*arraynames[]={
	 "@array/cleverwash", "@array/potch",
	 "@array/speed"     , "@array/dry",
	 "@array/stain"
    };
    LayoutInflater::from(getContext())->inflate("@layout/options",this);
    ViewGroup*vg=dynamic_cast<ViewGroup*>(getChildAt(0));
    for(int i=0;i<vg->getChildCount();i++){
	std::vector<int>values;
	std::vector<std::string>array;
	OptionPicker*op=dynamic_cast<OptionPicker*>(vg->getChildAt(i));
	vg->getChildAt(i)->setOnClickListener(std::bind(&WashOptionsWindow::onOptionClick,this,std::placeholders::_1));
	getContext()->getArray(arraynames[i],array);
        LOGD("%s size=%d",arraynames[i],array.size());
	if(op)op->setValuedName(values,array);
    }
}

void WashOptionsWindow::onOptionClick(View&v){
    ViewGroup*vg=v.getParent();
    for(int i=0;i<vg->getChildCount();i++){
        OptionPicker*op=dynamic_cast<OptionPicker*>(vg->getChildAt(i));
        op->showOptions(op==&v);
    }
    //invalidate();
}
