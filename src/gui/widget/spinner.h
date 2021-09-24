#ifndef __SPINNER_H__
#define __SPINNER_H__
#include <widget/absspinner.h>
#include <widget/listview.h>
namespace cdroid{

class Spinner:public AbsSpinner{
private:
    class SpinnerPopup {
    public:
        virtual void setAdapter(Adapter* adapter)=0;
        /**Show the popup */
        virtual void show(int textDirection, int textAlignment)=0;
        /* Dismiss the popup */
        virtual void dismiss()=0;
        /* @return true if the popup is showing, false otherwise. */
        virtual bool isShowing()=0;
        /**Set hint text to be displayed to the user. This should providea description
         * of the choice being made. @param hintText Hint text to set.*/
        virtual void setPromptText(const std::string& hintText)=0;
        virtual const std::string getHintText()=0;
        virtual int getVerticalOffset()=0;
        virtual void setVerticalOffset(int px)=0;
        virtual int getHorizontalOffset()=0;
        virtual void setHorizontalOffset(int px)=0;
        virtual void setBackgroundDrawable(Drawable* bg)=0;
        virtual Drawable* getBackground()=0;

        virtual void computeContentWidth()=0;
        virtual void setContentWidth(int width)=0;
        //virtual int measureContentWidth(Adapter*,Drawable*);
    };

    class DialogPopup:public SpinnerPopup{
    private:
        Window*mPopup;
        Spinner *mSpinner; 
        Adapter *mAdapter;
        std::string mPrompt;
    public:
        DialogPopup(Spinner*spinner);
        ~DialogPopup();
        void setAdapter(Adapter* adapter)override;
        void show(int textDirection, int textAlignment)override; 
        void dismiss()override;
        bool isShowing()override;
        void setPromptText(const std::string& hintText)override;
        const std::string getHintText()override;
        int getVerticalOffset()override;
        void setVerticalOffset(int px)override;
        int getHorizontalOffset()override;
        void setHorizontalOffset(int px)override;
        void setBackgroundDrawable(Drawable* bg)override;
        Drawable* getBackground()override;
        void computeContentWidth()override;
        void setContentWidth(int width)override;
    };

    class DropdownPopup:public SpinnerPopup{
    private:
        int mDropDownWidth;
        Spinner *mSpinner;
        Adapter *mAdapter;
        ListView*mListView;
    public:
        DropdownPopup(Spinner*spinner);
        ~DropdownPopup();
        void setAdapter(Adapter* adapter)override;
        void show(int textDirection, int textAlignment)override; 
        void dismiss()override;
        bool isShowing()override;
        void setPromptText(const std::string& hintText)override;
        const std::string getHintText()override;
        int getVerticalOffset()override;
        void setVerticalOffset(int px)override;
        int getHorizontalOffset()override;
        void setHorizontalOffset(int px)override;
        void setBackgroundDrawable(Drawable* bg)override;
        Drawable* getBackground()override;
        void computeContentWidth()override;
        void setContentWidth(int width)override;
        ListView*getListView()const;
    };
public:
    static constexpr int  MODE_DIALOG   = 0;
    static constexpr int  MODE_DROPDOWN = 1;
private:
    int  mGravity;
    Context*mPopupContext;
    bool mDisableChildrenWhenDisabled;
    void setUpChild(View* child, bool addChild);
    View* makeView(int position, bool addChild);
protected:
    int mDropDownWidth;
    SpinnerPopup *mPopup;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t, int w, int h);
    void layout(int delta, bool animate)override;
    void computeContentWidth();
public:
    Spinner(int w,int h,int mode=0);
    Spinner(Context*ctx,const AttributeSet&atts);
    Context* getPopupContext()const;
    int measureContentWidth(Adapter* adapter, Drawable* background);
    void setPopupBackgroundDrawable(Drawable* background);
    void setPopupBackgroundResource(const std::string& resId);
    Drawable* getPopupBackground();
    void setDropDownVerticalOffset(int pixels);
    int getDropDownVerticalOffset();
    void setDropDownHorizontalOffset(int pixels);
    int getDropDownHorizontalOffset();
    void setDropDownWidth(int pixels);
    int getDropDownWidth()const;
    View& setEnabled(bool enabled)override;
    void setGravity(int gravity);
    int getGravity()const;
    void setAdapter(Adapter*adapter)override;
    int getBaseline()override;
    void show(int textDirection, int textAlignment);
    bool onTouchEvent(MotionEvent& event)override;
};

}//namespace
#endif
