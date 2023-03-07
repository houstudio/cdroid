#include <washoptions.h>
#include <cdroid.h>
#include <R.h>

WashOptionsWindow::WashOptionsWindow(int options):Window(0,0,1280,480){
    LayoutInflater::from(getContext())->inflate("@layout/options",this);
    NumberPicker*np=(NumberPicker*)findViewById(w9::R::id::options);
    np->setMinValue(0);
    np->setMaxValue(4);
    LinearLayout*ll=(LinearLayout*)findViewById(w9::R::id::optionsContainer);
    const char*labels[]={
	"Button0",
	"Button1",
	"Button2",
	"Button3",
	"Button4",
	"Button5"
    };
    const int count=sizeof(labels)/sizeof(labels[0]);
    for(int i=0;i<count;i++){
	ToggleButton*opt=new ToggleButton(100,100);
        ll->addView(opt,new LinearLayout::LayoutParams(-1,32,1.f/count)).setId(100+i).setClickable(true);
	opt->setText(labels[i]);
	opt->setBackgroundResource("@drawable/optionbackground");
	opt->setOnClickListener(std::bind(&WashOptionsWindow::onOptionClick,this,std::placeholders::_1));
    }
    dynamic_cast<FrameLayout::LayoutParams*>(np->getLayoutParams())->setMarginStart(600);
    FrameLayout*frame=(FrameLayout*)findViewById(w9::R::id::content);
    frame->requestLayout();
}

void WashOptionsWindow::onOptionClick(View&v){
    ViewGroup*parent=v.getParent();
    const int count=parent->getChildCount();
    const bool checked=dynamic_cast<ToggleButton&>(v).isChecked();
    for(int i=0;i<count;i++){
        View*v=parent->getChildAt(i);
        dynamic_cast<ToggleButton*>(v)->setChecked(false);
    }
    dynamic_cast<ToggleButton&>(v).setChecked(checked);
    NumberPicker*np=(NumberPicker*)findViewById(w9::R::id::options);
    dynamic_cast<FrameLayout::LayoutParams*>(np->getLayoutParams())->setMarginStart(v.getLeft());
    FrameLayout*frame=(FrameLayout*)findViewById(w9::R::id::content);
    LOGD("set numberpicker to %d",v.getLeft());
    frame->requestLayout();
}
