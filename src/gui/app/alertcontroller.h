#ifndef __ALERT_CONTROLLER_H__
#define __ALERT_CONTROLLER_H__
#include <cdroid.h>
#include <app/dialoginterface.h>

namespace cdroid{

class AlertController{
public:
    DECLARE_UIEVENT(void,OnPrepareListViewListener,ListView&);
    class RecycleListView:public ListView {
    private:
        friend class AlertController;
        int mPaddingTopNoTitle;
        int mPaddingBottomNoButtons;
        bool mRecycleOnMeasure = true;
    protected:
        bool recycleOnMeasure();
    public:
        RecycleListView(Context* context,const AttributeSet& attrs);
        void setHasDecor(bool hasTitle, bool hasButtons);
    };
    class AlertParams {
    public:
        Context* mContext;
        LayoutInflater* mInflater;

        std::string mIconId;
        Drawable* mIcon;
        std::string mIconAttrId;
        std::string mTitle;
        View* mCustomTitleView;
        std::string mMessage;
        std::string mPositiveButtonText;
        DialogInterface::OnClickListener mPositiveButtonListener;
        std::string mNegativeButtonText;
        DialogInterface::OnClickListener mNegativeButtonListener;
        std::string mNeutralButtonText;
        DialogInterface::OnClickListener mNeutralButtonListener;
        bool mCancelable;
        DialogInterface::OnCancelListener mOnCancelListener;
        DialogInterface::OnDismissListener mOnDismissListener;
        DialogInterface::OnKeyListener mOnKeyListener;
        std::vector<std::string> mItems;
        ListAdapter* mAdapter;
        DialogInterface::OnClickListener mOnClickListener;
        std::string mViewLayoutResId;
        View* mView;
        int  mViewSpacingLeft;
        int  mViewSpacingTop;
        int  mViewSpacingRight;
        int  mViewSpacingBottom;
        bool mViewSpacingSpecified = false;
        std::vector<bool> mCheckedItems;
        bool mIsMultiChoice;
        bool mIsSingleChoice;
        int  mCheckedItem = -1;
        DialogInterface::OnMultiChoiceClickListener mOnCheckboxClickListener;
        //Cursor mCursor;
        std::string mLabelColumn;
        std::string mIsCheckedColumn;
        bool mForceInverseBackground;
        AdapterView::OnItemSelectedListener mOnItemSelectedListener;
        OnPrepareListViewListener mOnPrepareListViewListener;
        bool mRecycleOnMeasure = true;
    private:
        void createListView(AlertController* dialog);
    public:
        AlertParams(Context*);
        void apply(AlertController* dialog);
    };
private:
    Context*mContext;
    DialogInterface* mDialogInterface;
    Window* mWindow;

    std::string mTitle;
    View* mView;

    std::string mViewLayoutResId;

    int mViewSpacingLeft;
    int mViewSpacingTop;
    int mViewSpacingRight;
    int mViewSpacingBottom;
    bool mViewSpacingSpecified = false;

    Button* mButtonPositive;
    std::string mButtonPositiveText;
    View::OnClickListener mButtonPositiveListener;

    Button* mButtonNegative;
    std::string mButtonNegativeText;
    View::OnClickListener mButtonNegativeListener;

    Button* mButtonNeutral;
    std::string mButtonNeutralText;
    View::OnClickListener mButtonNeutralListener;

    std::string mIconId ;
    Drawable* mIcon;

    ImageView* mIconView;
    TextView* mTitleView;
    //MovementMethod mMessageMovementMethod;
    int  mMessageHyphenationFrequency;
    View* mCustomTitleView;

    bool mForceInverseBackground;

    Adapter* mAdapter;

    int mCheckedItem = -1;

    std::string mAlertDialogLayout;
    std::string mButtonPanelSideLayout;
    std::string mListLayout;
    std::string mMultiChoiceItemLayout;
    std::string mSingleChoiceItemLayout;
    std::string mListItemLayout;

    bool mShowTitle;
    int mButtonPanelLayoutHint;
private:
    void onButtonClick(DialogInterface::OnClickListener listener,View&v);
    static bool shouldCenterSingleButton(Context* context);
    const std::string& selectContentView();
    ViewGroup* resolvePanel(View* customPanel,View* defaultPanel);
    void setupView();
    void setupCustomContent(ViewGroup* customPanel);
    void centerButton(Button* button);
    void setBackground(const AttributeSet&,View* topPanel, View* contentPanel, View* customPanel,
            View* buttonPanel, bool hasTitle, bool hasCustomView, bool hasButtons); 
protected:
    std::string mMessage;
    ListView *  mListView;
    ScrollView* mScrollView;
    TextView *  mMessageView;
    AlertController(Context* context, DialogInterface* di, Window* window);
    static bool canTextInput(View* v);
    void setupTitle(ViewGroup* topPanel);
    void setupContent(ViewGroup*);
    void setupButtons(ViewGroup*);
public:
    static AlertController* create(Context* context, DialogInterface* di, Window* window);
    void installContent(AlertParams* params);
    void installContent();
    void setTitle(const std::string& title);
    void setCustomTitle(View* customTitleView);
    void setMessage(const std::string& message);
    void setView(const std::string&layoutResId);
    void setView(View* view);
    void setView(View* view, int viewSpacingLeft, int viewSpacingTop, int viewSpacingRight,int viewSpacingBottom);
    void setButton(int whichButton,const std::string&text,DialogInterface::OnClickListener listener);
    void setIcon(const std::string& resId);
    void setIcon(Drawable* icon);
    std::string getIconAttributeResId(const std::string&attrId);
    void setInverseBackgroundForced(bool forceInverseBackground);
    ListView* getListView();
    Button*getButton(int whichButton);
    bool onKeyDown(int keyCode, KeyEvent& event);
    bool onKeyUp(int keyCode, KeyEvent& event);
};
}//namespace 
#endif
