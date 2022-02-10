#include <widget/radiogroup.h>
#include <widget/radiobutton.h>
#include <cdlog.h>


namespace cdroid{

DECLARE_WIDGET(RadioGroup)
DECLARE_WIDGET2(RadioButton,"cdroid:attr/radioButtonStyle")

RadioGroup::RadioGroup(int w,int h):LinearLayout(w,h){
    setOrientation(VERTICAL);
    init();
}

RadioGroup::RadioGroup(Context* context,const AttributeSet& attrs)
    :LinearLayout(context,attrs){
    mCheckedId = attrs.getInt("checkedButton",NO_ID);

    setOrientation(attrs.getInt("orientation",VERTICAL));
    init();
}

LayoutParams* RadioGroup::generateLayoutParams(const AttributeSet& attrs)const {
    return new RadioGroupLayoutParams(getContext(), attrs);
}

bool RadioGroup::checkLayoutParams(const ViewGroup::LayoutParams* p)const {
    return dynamic_cast<const RadioGroupLayoutParams*>(p);
}

ViewGroup::LayoutParams* RadioGroup::generateDefaultLayoutParams()const {
    return new RadioGroupLayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
}

void RadioGroup::onRadioChecked(CompoundButton&c,bool checked){
    if(mProtectFromCheckedChange)return;
    mProtectFromCheckedChange =true;
    if (mCheckedId != -1) {
        setCheckedStateForView(mCheckedId, false);
    }
    LOGD("onRadioChecked %d",c.getId());
    mProtectFromCheckedChange = false;
    setCheckedId(c.getId());
}

void RadioGroup::OnHierarchyChange(ViewGroup&parent,View*c,bool add){
    if(add){
        auto fun=std::bind(&RadioGroup::onRadioChecked,this,
                std::placeholders::_1,std::placeholders::_2);
        ((RadioButton*)c)->setOnCheckedChangeWidgetListener(fun);
    }
}

void RadioGroup::init(){
    auto fun=std::bind(&RadioGroup::OnHierarchyChange,this,std::placeholders::_1,
                       std::placeholders::_2,std::placeholders::_3);
    LinearLayout::setOnHierarchyChangeListener(fun);
}

void RadioGroup::setOnCheckedChangeListener(CompoundButton::OnCheckedChangeListener listener){
    mOnCheckedChangeListener=listener;
}

int RadioGroup::getCheckedRadioButtonId()const{
    return mCheckedId;
}

void RadioGroup::setCheckedId(int id){
    mCheckedId = id;
    if (mOnCheckedChangeListener != nullptr) {
        mOnCheckedChangeListener((CompoundButton&)*this, mCheckedId);
    }
}

void RadioGroup::setCheckedStateForView(int viewId, bool checked){
    View* checkedView = findViewById(viewId);
    if (checkedView != nullptr && dynamic_cast<RadioButton*>(checkedView)) {
        ((RadioButton*) checkedView)->setChecked(checked);
    }
}

void RadioGroup::check(int id){
     if (id != -1 && (id == mCheckedId)) {
        return;
    }

    if (mCheckedId != -1) {
        setCheckedStateForView(mCheckedId, false);
    }

    if (id != -1) {
        setCheckedStateForView(id, true);
    }
    setCheckedId(id);
}

void RadioGroup::clearCheck(){
    check(-1);
}

View& RadioGroup::addView(View* child, int index,ViewGroup::LayoutParams* params){
    if (dynamic_cast<RadioButton*>(child)) {
        RadioButton* button = (RadioButton*) child;
        if (button->isChecked()) {
            mProtectFromCheckedChange = true;
            if (mCheckedId != -1) {
                setCheckedStateForView(mCheckedId, false);
            }
            mProtectFromCheckedChange = false;
            setCheckedId(button->getId());
        }
    }
    return LinearLayout::addView(child, index, params);
}

//////////////////////////////////////////////////////////////////////////////////
RadioGroup::LayoutParams::LayoutParams(Context*c,const AttributeSet&attrs)
:LinearLayoutParams(c,attrs){
}

RadioGroup::LayoutParams::LayoutParams(int w,int h)
:LinearLayoutParams(w,h){
}

RadioGroup::LayoutParams::LayoutParams(int w, int h, float initWeight)
    :LinearLayoutParams(w,h,initWeight){
}

RadioGroup::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& p)
    :LinearLayoutParams(p){
}

RadioGroup::LayoutParams::LayoutParams(const MarginLayoutParams& source)
    :LinearLayoutParams(source){
}

}
