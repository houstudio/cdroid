#ifndef __ALERT_CONTROLLER_H__
#define __ALERT_CONTROLLER_H__
#include <windows.h>
#include <app/dialoginterface.h>

namespace cdroid{

class AlertController{
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
    Message mButtonPositiveMessage;

    Button* mButtonNegative;
    std::string mButtonNegativeText;
    Message mButtonNegativeMessage;

    Button* mButtonNeutral;
    std::string mButtonNeutralText;
    Message mButtonNeutralMessage;


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
private:
    static bool shouldCenterSingleButton(Context* context);
    static AlertController* create(Context* context, DialogInterface* di, Window* window);
    const std::string& selectContentView();
    void setupView();
    void setupCustomContent(ViewGroup* customPanel);
protected:
    std::string mMessage;
    ListView *  mListView;
    ScrollView* mScrollView;
    TextView *  mMessageView;
    AlertController(Context* context, DialogInterface* di, Window* window);
    static bool canTextInput(View* v);
    void setupTitle(ViewGroup* topPanel);
public:
    void installContent(AlertParams* params);
    void installContent();
    void setTitle(const std::string& title);
    void setCustomTitle(View* customTitleView);
    void setMessage(const std::string& message);
    void setView(const std::string&layoutResId);
    void setView(View* view);
    void setView(View* view, int viewSpacingLeft, int viewSpacingTop, int viewSpacingRight,int viewSpacingBottom);
    void setButton(int whichButton,const std::string&text,DialogInterface::OnClickListener listener, Message msg);
    void setIcon(const std::string& resId);
    void setIcon(Drawable* icon);
    ListView* getListView();
    Button*getButton(int whichButton);
    bool onKeyDown(int keyCode, KeyEvent& event);
    bool onKeyUp(int keyCode, KeyEvent& event);
};
}//namespace 
#endif
