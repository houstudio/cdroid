#ifndef __COMPOUND_BUTTON_H__
#define __COMPOUND_BUTTON_H__
#include <widget/button.h>
#include <widget/checkable.h>
namespace cdroid{
class CompoundButton:public Button,public Checkable{
public:
    DECLARE_UIEVENT(void,OnCheckedChangeListener,CompoundButton&view,bool);
private: 
    bool mChecked;
    bool mBroadcasting;
    bool mCheckedFromResource;
    Drawable* mButtonDrawable;
    OnCheckedChangeListener mOnCheckedChangeListener;
    OnCheckedChangeListener mOnCheckedChangeWidgetListener;
    void initCompoundButton();
protected:
    std::vector<int>onCreateDrawableState()override;
    int getHorizontalOffsetForDrawables()const override;
    void drawableStateChanged()override;
    bool verifyDrawable(Drawable* who)const override;
    void onDraw(Canvas&canvas)override;
    virtual void doSetChecked(bool);
public:
    CompoundButton(const std::string&txt,int width,int height);
    CompoundButton(Context*ctx,const AttributeSet&attrs);
    ~CompoundButton()override;
    void setButtonDrawable(const std::string&resid);
    void setButtonDrawable(Drawable*d);
    bool performClick()override;
    Drawable* getButtonDrawable()const;
    void jumpDrawablesToCurrentState()override;

    //inerited from Checkable
    //void setChecked(bool checked)override;
    //bool isChecked() override;
    //void toggle()override;

    void setOnCheckedChangeListener(OnCheckedChangeListener listener);
    /*OnCheckedChangeWidgetListener internal use(for radiogroup...)*/
    void setOnCheckedChangeWidgetListener(OnCheckedChangeListener listener);
    void drawableHotspotChanged(float x,float y)override;
};
}
#endif
