#ifndef __ALERT_DIALOG_H__
#define __ALERT_DIALOG_H__
#include <widget/button.h>
#include <widget/listview.h>
#include <app/dialog.h>
#include <app/alertcontroller.h>

namespace cdroid{
	
class AlertDialog :public Dialog{///DialogInterface{
public:
    static constexpr int LAYOUT_HINT_NONE = 0;
    static constexpr int LAYOUT_HINT_SIDE = 1;
    class Builder{
    private:
        AlertController::AlertParams* P;
    public:
        Builder(Context* context);
        ~Builder(); 
        Context* getContext();
        Builder& setTitle(const std::string& title);
        Builder& setCustomTitle(View* customTitleView);
        Builder& setMessage(const std::string&messageId);
        Builder& setIcon(const std::string&iconId);
        Builder& setIcon(Drawable*icon);
        Builder& setPositiveButton(const std::string& textId,OnClickListener listener);
        Builder& setNegativeButton(const std::string& textId, OnClickListener listener);
        Builder& setNeutralButton(const std::string& textId, OnClickListener listener);
        Builder& setCancelable(bool cancelable);
        Builder& setOnCancelListener(OnCancelListener onCancelListener);
        Builder& setOnDismissListener(OnDismissListener onDismissListener);
        Builder& setOnKeyListener(OnKeyListener onKeyListener);
        Builder& setItems(const std::string& itemsId,OnClickListener listener);
        Builder& setItems(const std::vector<std::string>&items, OnClickListener listener);
        Builder& setAdapter(ListAdapter* adapter,OnClickListener listener);
        Builder& setMultiChoiceItems(const std::string&itemsId,const std::vector<bool>& checkedItems,
                OnMultiChoiceClickListener listener);
        Builder& setMultiChoiceItems(const std::vector<std::string>&items, const std::vector<bool>& checkedItems,
                OnMultiChoiceClickListener listener);
        Builder& setSingleChoiceItems(const std::string&itemsId, int checkedItem, OnClickListener listener);
        Builder& setSingleChoiceItems(const std::vector<std::string>&items, int checkedItem,OnClickListener listener);
        Builder& setSingleChoiceItems(ListAdapter* adapter, int checkedItem,OnClickListener listener);
        Builder& setOnItemSelectedListener(AdapterView::OnItemSelectedListener listener);
        Builder& setView(const std::string&layoutResId);
        Builder& setView(View* view);
        Builder& setRecycleOnMeasureEnabled(bool enabled);
        AlertDialog* create();
        AlertDialog* show();
    };
protected:
    class AlertController* mAlert;
protected:
    AlertDialog(Context*ctx);
    AlertDialog(Context*ctx,const std::string&resid); 
    AlertDialog(Context*ctx,bool cancelable,OnCancelListener listener);
    void onCreate()override;
public:
    Button*getButton(int whichButton);
    ListView* getListView();
    void setTitle(const std::string& title);
    void setCustomTitle(View*customTitleView);
    virtual void setMessage(const std::string& message);
    virtual void setView(View* view);
    void setView(View* view, int viewSpacingLeft, int viewSpacingTop, int viewSpacingRight,
            int viewSpacingBottom);
    void setButton(int whichButton,const std::string&text, OnClickListener listener);
    void setIcon(const std::string&);
    void setIcon(Drawable*);
    void setInverseBackgroundForced(bool forceInverseBackground);
    bool onKeyDown(int keyCode, KeyEvent& event)override;
    bool onKeyUp(int keyCode, KeyEvent& event)override;
};

}//namespace 
#endif 
