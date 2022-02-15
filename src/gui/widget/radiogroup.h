#ifndef __RADIOGROUP_H__
#define __RADIOGROUP_H__
#include <widget/linearlayout.h>
#include <widget/radiobutton.h>
namespace cdroid{


class RadioGroup: public LinearLayout{
public:
    class LayoutParams:public LinearLayout::LayoutParams{
    public:
        LayoutParams(Context*c,const AttributeSet&);
        LayoutParams(int w,int h);
        LayoutParams(int w, int h, float initWeight);
        LayoutParams(const ViewGroup::LayoutParams& p);
        LayoutParams(const MarginLayoutParams& source);
    };
private:
    int mCheckedId;
    bool mProtectFromCheckedChange;
    CompoundButton::OnCheckedChangeListener mOnCheckedChangeListener;
    void init();
    void setCheckedId(int id);
    void setCheckedStateForView(int viewId, bool checked);
    void onRadioChecked(CompoundButton&c,bool checked);
    void OnHierarchyChange(ViewGroup&parent,View*c,bool add);
protected:
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    ViewGroup::LayoutParams* generateDefaultLayoutParams()const override;
public:
    RadioGroup(int w,int h);
    RadioGroup(Context* context,const AttributeSet& attrs);
    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
    View& addView(View* child, int index,ViewGroup::LayoutParams* params)override;
    int getCheckedRadioButtonId()const;
    void setOnCheckedChangeListener(CompoundButton::OnCheckedChangeListener listener);
    void check(int id);
    void clearCheck();
};
}//namepace

#endif
