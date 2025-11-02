#ifndef __CHECKED_TEXTVIEW_H__
#define __CHECKED_TEXTVIEW_H__
#include <widget/textview.h>
#include <widget/checkable.h>
namespace cdroid{

class CheckedTextView:public TextView,public Checkable{
private:
    bool mChecked;

    std::string mCheckMarkResource;
    Drawable* mCheckMarkDrawable;
    const ColorStateList* mCheckMarkTintList;
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
    void doSetChecked(bool checked);
    bool verifyDrawable(Drawable* who)const override;
    void internalSetPadding(int left, int top, int right, int bottom)override;
    void onDetachedFromWindowInternal()override;
    void onDraw(Canvas& canvas)override;
    std::vector<int>onCreateDrawableState(int)override;
    void drawableStateChanged()override;
    void drawableHotspotChanged(float x, float y)override;
public:
    CheckedTextView(Context* context,const AttributeSet& attrs);
    ~CheckedTextView()override;
#ifndef FUNCTION_AS_CHECKABLE
    void toggle();
    bool isChecked()const;
    void setChecked(bool checked);
#endif
    Drawable* getCheckMarkDrawable()const;
    void setCheckMarkDrawable(const std::string&resId);
    void setCheckMarkDrawable(Drawable* d);
    void setCheckMarkTintList(const ColorStateList*tint);
    const ColorStateList* getCheckMarkTintList()const;
    void setVisibility(int visibility)override;
    void jumpDrawablesToCurrentState()override;
    void onRtlPropertiesChanged(int layoutDirection)override;

    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityEventInternal(AccessibilityEvent& event)override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
};
}//endof namespace

#endif
