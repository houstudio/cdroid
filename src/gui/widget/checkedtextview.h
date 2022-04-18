#ifndef __CHECKED_TEXTVIEW_H__
#define __CHECKED_TEXTVIEW_H__
#include <widget/textview.h>
#include <widget/checkable.h>
namespace cdroid{

class CheckedTextView:public TextView,Checkable{
private:
    bool mChecked;

    std::string mCheckMarkResource;
    Drawable* mCheckMarkDrawable;
    ColorStateList* mCheckMarkTintList = nullptr;
    //BlendMode mCheckMarkBlendMode = null;
    bool mHasCheckMarkTint = false;
    bool mHasCheckMarkTintMode = false;

    int mBasePadding;
    int mCheckMarkWidth;
    int mCheckMarkGravity = Gravity::END;

    bool mNeedRequestlayout;
private:
    void setCheckMarkDrawableInternal(Drawable* d,const std::string&resId);
    void applyCheckMarkTint();
    void updatePadding();
    void setBasePadding(bool checkmarkAtStart);
    bool isCheckMarkAtStart();
protected:
    ~CheckedTextView();
    bool verifyDrawable(Drawable* who)const override;
    void internalSetPadding(int left, int top, int right, int bottom);
    void onDraw(Canvas& canvas)override;
    std::vector<int>onCreateDrawableState()override;
    void drawableStateChanged()override;
    void drawableHotspotChanged(float x, float y)override;
public:
    CheckedTextView(Context* context,const AttributeSet& attrs);
    void toggle();
    bool isChecked()const;
    void setChecked(bool checked);
    Drawable* getCheckMarkDrawable()const;
    void setCheckMarkDrawable(const std::string&resId);
    void setCheckMarkDrawable(Drawable* d);
    void setCheckMarkTintList(ColorStateList*tint);
    ColorStateList* getCheckMarkTintList()const;
    View& setVisibility(int visibility)override;     
    void jumpDrawablesToCurrentState()override;
    void onRtlPropertiesChanged(int layoutDirection)override;
};
}//endof namespace

#endif
