#include <app/alertcontroller.h>
#include <app/alertdialog.h>
#include <widget/layoutinflater.h>

namespace cdroid{

bool AlertController::shouldCenterSingleButton(Context* context){
    return true;
}

AlertController* AlertController::create(Context* context, DialogInterface* di, Window* window){
    return new AlertController(context, di, window);
}

AlertController::AlertController(Context* context, DialogInterface* di, Window* window){
    mContext = context;
    mDialogInterface = di;
    mWindow = window;
    mHandler = new ButtonHandler(di);

    TypedArray a = context.obtainStyledAttributes(null,
            R.styleable.AlertDialog, R.attr.alertDialogStyle, 0);

    mAlertDialogLayout = a.getResourceId(
            R.styleable.AlertDialog_layout, R.layout.alert_dialog);
    mButtonPanelSideLayout = a.getResourceId(
            R.styleable.AlertDialog_buttonPanelSideLayout, 0);
    mListLayout = a.getResourceId(
            R.styleable.AlertDialog_listLayout, R.layout.select_dialog);

    mMultiChoiceItemLayout = a.getResourceId(
            R.styleable.AlertDialog_multiChoiceItemLayout,
            R.layout.select_dialog_multichoice);
    mSingleChoiceItemLayout = a.getResourceId(
            R.styleable.AlertDialog_singleChoiceItemLayout,
            R.layout.select_dialog_singlechoice);
    mListItemLayout = a.getResourceId(
            R.styleable.AlertDialog_listItemLayout,
            R.layout.select_dialog_item);
    mShowTitle = a.getBoolean(R.styleable.AlertDialog_showTitle, true);

    a.recycle();

    /* We use a custom title so never request a window title */
    //window.requestFeature(Window.FEATURE_NO_TITLE);
}

bool AlertController::canTextInput(View* v) {
    if (dynamic_cast<EditText*>(v)){//v->onCheckIsTextEditor()) {
        return true;
    }

    if (dynamic_cast<ViewGroup*>(v)==nullptr) {
        return false;
    }

    ViewGroup* vg = (ViewGroup*)v;
    int i = vg->getChildCount();
    while (i > 0) {
        i--;
        v = vg->getChildAt(i);
        if (canTextInput(v)) {
            return true;
        }
    }
    return false;
}

void AlertController::installContent(AlertParams* params) {
    params->apply(this);
    installContent();
}

void AlertController::installContent() {
    int contentView = selectContentView();
    mWindow->setContentView(contentView);
    setupView();
}

const std::string& AlertController::selectContentView() {
    if (mButtonPanelSideLayout.empty()) {
        return mAlertDialogLayout;
    }
    if (mButtonPanelLayoutHint == AlertDialog::LAYOUT_HINT_SIDE) {
        return mButtonPanelSideLayout;
    }
    // TODO: use layout hint side for long messages/lists
    return mAlertDialogLayout;
 
}

void AlertController::setTitle(const std::string& title) {
    mTitle = title;
    if (mTitleView ) {
        mTitleView->setText(title);
    }
}

/**@see AlertDialog.Builder#setCustomTitle(View)*/
void AlertController::setCustomTitle(View* customTitleView) {
    mCustomTitleView = customTitleView;
}

void AlertController::setMessage(const std::string& message) {
    mMessage = message;
    if (mMessageView) {
        mMessageView->setText(message);
    }
}

void AlertController::setView(const std::string&layoutResId) {
    mView = nullptr;
    mViewLayoutResId = layoutResId;
    mViewSpacingSpecified = false;
}

/**
  * Set the view to display in the dialog.
 */
void AlertController::setView(View* view) {
    mView = view;
    mViewLayoutResId.clear();
    mViewSpacingSpecified = false;
}

void AlertController::setView(View* view, int viewSpacingLeft, int viewSpacingTop, int viewSpacingRight,int viewSpacingBottom){
    mView = view;
    mViewLayoutResId.clear();
    mViewSpacingSpecified = true;
    mViewSpacingLeft = viewSpacingLeft;
    mViewSpacingTop = viewSpacingTop;
    mViewSpacingRight = viewSpacingRight;
    mViewSpacingBottom = viewSpacingBottom;
}

void AlertController::setButton(int whichButton,const std::string&text,DialogInterface::OnClickListener listener, Message msg){
    switch (whichButton) {

    case DialogInterface::BUTTON_POSITIVE:
         mButtonPositiveText = text;
         mButtonPositiveMessage = msg;
         break;

    case DialogInterface::BUTTON_NEGATIVE:
         mButtonNegativeText = text;
         mButtonNegativeMessage = msg;
         break;

    case DialogInterface::BUTTON_NEUTRAL:
         mButtonNeutralText = text;
         mButtonNeutralMessage = msg;
         break;

    default: LOGE("Button %d does not exist",whichButton);
    }
}

void AlertController::setIcon(const std::string& resId){
    mIcon = nullptr;
    mIconId = resId;

    if (mIconView != nullptr) {
        if (resId.size()) {
            mIconView->setVisibility(View::VISIBLE);
            mIconView->setImageResource(mIconId);
        } else {
            mIconView->setVisibility(View::GONE);
        }
    }
}

void AlertController::setIcon(Drawable* icon) {
    mIcon = icon;
    mIconId.clear();

    if (mIconView != nullptr) {
        if (icon != nullptr) {
            mIconView->setVisibility(View::VISIBLE);
            mIconView->setImageDrawable(icon);
        } else {
            mIconView->setVisibility(View::GONE);
        }
    }
}

ListView* AlertController::getListView() {
    return mListView;
}

Button* AlertController::getButton(int whichButton) {
    switch (whichButton) {
    case DialogInterface::BUTTON_POSITIVE:
        return mButtonPositive;
    case DialogInterface::BUTTON_NEGATIVE:
        return mButtonNegative;
    case DialogInterface::BUTTON_NEUTRAL:
        return mButtonNeutral;
    default:   return nullptr;
    }
}

bool AlertController::onKeyDown(int keyCode, KeyEvent& event){
    return mScrollView && mScrollView->executeKeyEvent(event);
}

bool AlertController::onKeyUp(int keyCode, KeyEvent& event){
    return mScrollView && mScrollView->executeKeyEvent(event);
}

void AlertController::setupView() {
    View* parentPanel = mWindow->findViewById(R.id.parentPanel);
    View* defaultTopPanel = parentPanel->findViewById(R.id.topPanel);
    View* defaultContentPanel= parentPanel->findViewById(R.id.contentPanel);
    View* defaultButtonPanel = parentPanel->findViewById(R.id.buttonPanel);

    // Install custom content before setting up the title or buttons so
    // that we can handle panel overrides.
    ViewGroup* customPanel = (ViewGroup*) parentPanel->findViewById(R.id.customPanel);
    setupCustomContent(customPanel);

    View* customTopPanel    = customPanel->findViewById(R.id.topPanel);
    View* customContentPanel= customPanel->findViewById(R.id.contentPanel);
    View* customButtonPanel = customPanel->findViewById(R.id.buttonPanel);

    // Resolve the correct panels and remove the defaults, if needed.
    ViewGroup* topPanel    = resolvePanel(customTopPanel , defaultTopPanel);
    ViewGroup* contentPanel= resolvePanel(customContentPanel, defaultContentPanel);
    ViewGroup* buttonPanel = resolvePanel(customButtonPanel , defaultButtonPanel);

    setupContent(contentPanel);
    setupButtons(buttonPanel);
    setupTitle(topPanel);

    bool hasCustomPanel = customPanel && customPanel->getVisibility()!= View::GONE;
    bool hasTopPanel    = topPanel    && topPanel->getVisibility()   != View::GONE;
    bool hasButtonPanel = buttonPanel && buttonPanel->getVisibility()!= View::GONE;

    // Only display the text spacer if we don't have buttons.
    if (!hasButtonPanel) {
        if (contentPanel != nullptr) {
            View* spacer = contentPanel->findViewById(R.id.textSpacerNoButtons);
            if (spacer != nullptr) {
                spacer->setVisibility(View::VISIBLE);
            }
        }
        mWindow->setCloseOnTouchOutsideIfNotSet(true);
    }

    if (hasTopPanel) {
        // Only clip scrolling content to padding if we have a title.
        if (mScrollView != nullptr) {
            mScrollView->setClipToPadding(true);
        }

        // Only show the divider if we have a title.
        View* divider = nullptr;
        if (mMessage.size() || mListView != nullptr || hasCustomPanel) {
            if (!hasCustomPanel) {
                divider = topPanel->findViewById(R.id.titleDividerNoCustom);
            }
            if (divider == nullptr) {
                divider = topPanel->findViewById(R.id.titleDivider);
            }

        } else {
            divider = topPanel->findViewById(R.id.titleDividerTop);
        }

        if (divider != nullptr) {
            divider->setVisibility(View::VISIBLE);
        }
    } else {
        if (contentPanel != nullptr) {
            View* spacer = contentPanel->findViewById(R.id.textSpacerNoTitle);
            if (spacer != nullptr) {
                spacer->setVisibility(View::VISIBLE);
            }
        }
    }

    if (dynamic_cast<ListView*>(mListView)) {
        ((ListView*)mListView)->setHasDecor(hasTopPanel, hasButtonPanel);
    }

    // Update scroll indicators as needed.
    if (!hasCustomPanel) {
        View* content = mListView != nullptr ? mListView : mScrollView;
        if (content != nullptr) {
            int indicators = (hasTopPanel ? View::SCROLL_INDICATOR_TOP : 0)
                    | (hasButtonPanel ? View::SCROLL_INDICATOR_BOTTOM : 0);
            content->setScrollIndicators(indicators,View::SCROLL_INDICATOR_TOP | View::SCROLL_INDICATOR_BOTTOM);
        }
    }

    TypedArray a = mContext.obtainStyledAttributes(
            null, R.styleable.AlertDialog, R.attr.alertDialogStyle, 0);
    setBackground(a, topPanel, contentPanel, customPanel, buttonPanel,
            hasTopPanel, hasCustomPanel, hasButtonPanel);
    a.recycle();
}

void AlertController::setupCustomContent(ViewGroup* customPanel){
    View* customView=nullptr;
    if (mView != nullptr) {
        customView = mView;
    } else if (mViewLayoutResId.size()) {
        LayoutInflater* inflater = LayoutInflater::from(mContext);
        customView = inflater->inflate(mViewLayoutResId, customPanel, false);
        delete inflater;
    } 
    bool hasCustomView = customView != nullptr;
    if (!hasCustomView || !canTextInput(customView)) {
        //mWindow->setFlags(WindowManager::LayoutParams::FLAG_ALT_FOCUSABLE_IM,WindowManager::LayoutParams::FLAG_ALT_FOCUSABLE_IM);
    }

    if (hasCustomView) {
        FrameLayout* custom = (FrameLayout*) mWindow->findViewById(R.id.custom);
        custom->addView(customView, new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT));

        if (mViewSpacingSpecified) {
            custom->setPadding(mViewSpacingLeft, mViewSpacingTop, mViewSpacingRight, mViewSpacingBottom);
        }

        if (mListView != nullptr) {
            ((LinearLayout::LayoutParams*) customPanel->getLayoutParams())->weight = 0;
        }
    } else {
        customPanel->setVisibility(View::GONE);
    }
}

void AlertController::setupTitle(ViewGroup topPanel) {
    if (mCustomTitleView && mShowTitle) {
        // Add the custom title view directly to the topPanel layout
        LayoutParams* lp = new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT);

        topPanel->addView(mCustomTitleView, 0, lp);

            // Hide the title template
        View* titleTemplate = mWindow->findViewById(R.id.title_template);
            titleTemplate->setVisibility(View::GONE);
    } else {
        mIconView = (ImageView*) mWindow->findViewById(R.id.icon);

        bool hasTextTitle = !TextUtils.isEmpty(mTitle);
        if (hasTextTitle && mShowTitle) {
            // Display the title if a title is supplied, else hide it.
            mTitleView = (TextView*) mWindow->findViewById(R.id.alertTitle);
            mTitleView->setText(mTitle);

            // Do this last so that if the user has supplied any icons we
            // use them instead of the default ones. If the user has
            // specified 0 then make it disappear.
            if (mIconId != 0) {
                 mIconView->setImageResource(mIconId);
            } else if (mIcon) {
                 mIconView->setImageDrawable(mIcon);
            } else {
                // Apply the padding from the icon to ensure the title is
                // aligned correctly.
                mTitleView->setPadding(mIconView.getPaddingLeft(),
                            mIconView->getPaddingTop(),
                            mIconView->getPaddingRight(),
                            mIconView->getPaddingBottom());
                mIconView->setVisibility(View::GONE);
            }
        } else {
            // Hide the title template
            View* titleTemplate = mWindow->findViewById(R.id.title_template);
            titleTemplate.setVisibility(View::GONE);
            mIconView->setVisibility(View::GONE);
            topPanel->setVisibility(View::GONE);
        }
    }
}

}//namespace

