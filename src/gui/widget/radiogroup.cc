#include <widget/radiogroup.h>
#include <widget/radiobutton.h>
#include <porting/cdlog.h>
#include <utils/textutils.h>

namespace cdroid{

DECLARE_WIDGET(RadioGroup)

RadioGroup::RadioGroup(int w,int h):LinearLayout(w,h){
    init();
    setOrientation(VERTICAL);
}

RadioGroup::RadioGroup(Context* context,const AttributeSet& attrs)
    :LinearLayout(context,attrs){
    init();
    const int value = attrs.getResourceId("checkedButton",View::NO_ID);
    if(value!=View::NO_ID){
        mCheckedId = value;
        mInitialCheckedId = value;
    }
    const int index = attrs.getInt("orientation",std::unordered_map<std::string,int>{
             {"horizontal",HORIZONTAL},
             {"vertical",VERTICAL} },VERTICAL);
    setOrientation(index);
}

LinearLayout::LayoutParams* RadioGroup::generateLayoutParams(const AttributeSet& attrs)const {
    return new LayoutParams(getContext(), attrs);
}

bool RadioGroup::checkLayoutParams(const ViewGroup::LayoutParams* p)const {
    return dynamic_cast<const LayoutParams*>(p);
}

LinearLayout::LayoutParams* RadioGroup::generateDefaultLayoutParams()const {
    return new LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
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

void RadioGroup::onChildViewAdded(View& parent, View* child){
    if((&parent==this)&&dynamic_cast<RadioButton*>(child)){
	    int id = child->getId();
	    if(id==View::NO_ID){
	        id = child->generateViewId();
	        child->setId(id);
	    }
	    ((RadioButton*)child)->setOnCheckedChangeWidgetListener(mChildOnCheckedChangeListener);
        if(mOnHierarchyChangeListener.onChildViewAdded)
            mOnHierarchyChangeListener.onChildViewAdded(parent, child);
    }
}

void RadioGroup::onChildViewRemoved(View& parent, View* child){
    if((&parent==this)&&dynamic_cast<RadioButton*>(child)){
	    ((RadioButton*)child)->setOnCheckedChangeWidgetListener(nullptr);
    }
    if(mOnHierarchyChangeListener.onChildViewRemoved)
        mOnHierarchyChangeListener.onChildViewRemoved(parent, child);
}

void RadioGroup::init(){
    ViewGroup::OnHierarchyChangeListener lhs;
    mCheckedId = View::NO_ID;
    mInitialCheckedId = false;
    mChildOnCheckedChangeListener=[this](CompoundButton&view,bool checked){
        onRadioChecked(view,checked);
    };
    lhs.onChildViewAdded  = [this](View&parent,View*child){
        onChildViewAdded(parent,child);
    };
    lhs.onChildViewRemoved= [this](View&parent,View*child){
        onChildViewRemoved(parent,child);
    };
    LinearLayout::setOnHierarchyChangeListener(lhs);
}

void RadioGroup::setOnHierarchyChangeListener(const ViewGroup::OnHierarchyChangeListener& listener) {
        // the user listener is delegated to our pass-through listener
    mOnHierarchyChangeListener = listener;
}

void RadioGroup::setOnCheckedChangeListener(const CompoundButton::OnCheckedChangeListener& listener){
    mOnCheckedChangeListener = listener;
}

int RadioGroup::getCheckedRadioButtonId()const{
    return mCheckedId;
}

std::string RadioGroup::getAccessibilityClassName()const{
    return "RadioGroup";
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
    if ((id != -1) && (id == mCheckedId)) {
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

void RadioGroup::onFinishInflate() {
    LinearLayout::onFinishInflate();

    // checks the appropriate radio button as requested in the XML file
    if (mCheckedId != -1) {
        mProtectFromCheckedChange = true;
        setCheckedStateForView(mCheckedId, true);
        mProtectFromCheckedChange = false;
        setCheckedId(mCheckedId);
    }
}

void RadioGroup::addView(View* child, int index,ViewGroup::LayoutParams* params){
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
    LinearLayout::addView(child, index, params);
}

void RadioGroup::onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info) {
    LinearLayout::onInitializeAccessibilityNodeInfo(info);
    if (getOrientation() == HORIZONTAL) {
        info.setCollectionInfo(AccessibilityNodeInfo::CollectionInfo::obtain(1,
                getVisibleChildWithTextCount(), false,
                AccessibilityNodeInfo::CollectionInfo::SELECTION_MODE_SINGLE));
    } else {
        info.setCollectionInfo(
                AccessibilityNodeInfo::CollectionInfo::obtain(getVisibleChildWithTextCount(),
                1, false,
                AccessibilityNodeInfo::CollectionInfo::SELECTION_MODE_SINGLE));
    }
}

int RadioGroup::getVisibleChildWithTextCount() const{
    int count = 0;
    for (int i = 0; i < getChildCount(); i++) {
        if (dynamic_cast<RadioButton*>(getChildAt(i))) {
            if (isVisibleWithText((RadioButton*) getChildAt(i))) {
                count++;
            }
        }
    }
    return count;
}

int RadioGroup::getIndexWithinVisibleButtons(View* child) const{
    if (dynamic_cast<RadioButton*>(child)==nullptr) {
        return -1;
    }
    int index = 0;
    for (int i = 0; i < getChildCount(); i++) {
        if (dynamic_cast<RadioButton*>(getChildAt(i))) {
            RadioButton* button = (RadioButton*) getChildAt(i);
            if (button == child) {
                return index;
            }
            if (isVisibleWithText(button)) {
                index++;
            }
        }
    }
    return -1;
}

 bool RadioGroup::isVisibleWithText(RadioButton* button) const{
    return button->getVisibility() == VISIBLE && !TextUtils::isEmpty(button->getText());
}

//////////////////////////////////////////////////////////////////////////////////
RadioGroup::LayoutParams::LayoutParams(Context*c,const AttributeSet&attrs)
    :LinearLayout::LayoutParams(c,attrs){
}

RadioGroup::LayoutParams::LayoutParams(int w,int h)
    :LinearLayout::LayoutParams(w,h){
}

RadioGroup::LayoutParams::LayoutParams(int w, int h, float initWeight)
    :LinearLayout::LayoutParams(w,h,initWeight){
}

RadioGroup::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& p)
    :LinearLayout::LayoutParams(p){
}

RadioGroup::LayoutParams::LayoutParams(const MarginLayoutParams& source)
    :LinearLayout::LayoutParams(source){
}

}
