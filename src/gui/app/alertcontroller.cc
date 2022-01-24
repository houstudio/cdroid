#include <app/alertcontroller.h>
#include <app/alertdialog.h>
#include <widget/layoutinflater.h>
#include <widget/R.h>

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

    AttributeSet atts;
    //TypedArray a = context.obtainStyledAttributes(null, R.styleable.AlertDialog, R.attr.alertDialogStyle, 0);

    mAlertDialogLayout = atts.getString("layout","cdroid:layout/alert_dialog.xml");
    mButtonPanelSideLayout = atts.getString("buttonPanelSideLayout");
    mListLayout = atts.getString("listLayout","cdroid:layout/select_dialog.xml");

    mMultiChoiceItemLayout = atts.getString("multiChoiceItemLayout","cdroid:layout/select_dialog_multichoice.xml");
    mSingleChoiceItemLayout= atts.getString("singleChoiceItemLayout","cdroid:layout/select_dialog_singlechoice.xml");
    mListItemLayout = atts.getString("listItemLayout","cdroid:layout/select_dialog_item.xml");
    mShowTitle = atts.getBoolean("showTitle", true);

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
    const std::string contentView = selectContentView();
    //mWindow->setContentView(contentView);
    LayoutInflater::from(mContext)->inflate(contentView,mWindow,true);
    setupView();
}

const std::string& AlertController::selectContentView() {
    if (mButtonPanelSideLayout.empty()) {
        return mAlertDialogLayout;
    }
    if (mButtonPanelLayoutHint == (int)AlertDialog::LAYOUT_HINT_SIDE) {
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

void AlertController::setButton(int whichButton,const std::string&text,DialogInterface::OnClickListener listener){
    switch (whichButton) {

    case DialogInterface::BUTTON_POSITIVE:
         mButtonPositiveText = text;
         //mButtonPositiveMessage = msg;
         break;

    case DialogInterface::BUTTON_NEGATIVE:
         mButtonNegativeText = text;
         //mButtonNegativeMessage = msg;
         break;

    case DialogInterface::BUTTON_NEUTRAL:
         mButtonNeutralText = text;
         //mButtonNeutralMessage = msg;
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

std::string AlertController::getIconAttributeResId(const std::string&attrId){
    return "";
}

void AlertController::setInverseBackgroundForced(bool forceInverseBackground){
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

ViewGroup* AlertController::resolvePanel(View* customPanel,View* defaultPanel){
    if(customPanel==nullptr){
        return (ViewGroup*)defaultPanel;
    }
    if(defaultPanel){
        ViewGroup*parent=defaultPanel->getParent();
        parent->removeView(defaultPanel);
    }
    return (ViewGroup*)customPanel;
}

void AlertController::setupView() {
    View* parentPanel = mWindow->findViewById(R::id::parentPanel);
    View* defaultTopPanel = parentPanel->findViewById(R::id::topPanel);
    View* defaultContentPanel= parentPanel->findViewById(R::id::contentPanel);
    View* defaultButtonPanel = parentPanel->findViewById(R::id::buttonPanel);

    // Install custom content before setting up the title or buttons so
    // that we can handle panel overrides.
    ViewGroup* customPanel = (ViewGroup*) parentPanel->findViewById(R::id::customPanel);
    setupCustomContent(customPanel);

    View* customTopPanel    = customPanel->findViewById(R::id::topPanel);
    View* customContentPanel= customPanel->findViewById(R::id::contentPanel);
    View* customButtonPanel = customPanel->findViewById(R::id::buttonPanel);

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
            View* spacer = contentPanel->findViewById(R::id::textSpacerNoButtons);
            if (spacer != nullptr) {
                spacer->setVisibility(View::VISIBLE);
            }
        }
        //mWindow->setCloseOnTouchOutsideIfNotSet(true);
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
                divider = topPanel->findViewById(R::id::titleDividerNoCustom);
            }
            if (divider == nullptr) {
                divider = topPanel->findViewById(R::id::titleDivider);
            }

        } else {
            divider = topPanel->findViewById(R::id::titleDividerTop);
        }

        if (divider != nullptr) {
            divider->setVisibility(View::VISIBLE);
        }
    } else {
        if (contentPanel != nullptr) {
            View* spacer = contentPanel->findViewById(R::id::textSpacerNoTitle);
            if (spacer != nullptr) {
                spacer->setVisibility(View::VISIBLE);
            }
        }
    }

    if (dynamic_cast<ListView*>(mListView)) {
        //((ListView*)mListView)->setHasDecor(hasTopPanel, hasButtonPanel);
    }

    // Update scroll indicators as needed.
    if (!hasCustomPanel) {
        View* content = mListView != nullptr ? (View*)mListView : (View*)mScrollView;
        if (content != nullptr) {
            int indicators = (hasTopPanel ? View::SCROLL_INDICATOR_TOP : 0)
                    | (hasButtonPanel ? View::SCROLL_INDICATOR_BOTTOM : 0);
            content->setScrollIndicators(indicators,View::SCROLL_INDICATOR_TOP | View::SCROLL_INDICATOR_BOTTOM);
        }
    }

    AttributeSet atts;
    //mContext->obtainStyledAttributes("   TypedArray a = mContext.obtainStyledAttributes(
            //null, R.styleable.AlertDialog, R.attr.alertDialogStyle, 0);
    setBackground(atts, topPanel, contentPanel, customPanel, buttonPanel,
            hasTopPanel, hasCustomPanel, hasButtonPanel);
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
        FrameLayout* custom = (FrameLayout*) mWindow->findViewById(R::id::custom);
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

void AlertController::setupTitle(ViewGroup* topPanel) {
    if (mCustomTitleView && mShowTitle) {
        // Add the custom title view directly to the topPanel layout
        LayoutParams* lp = new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT);

        topPanel->addView(mCustomTitleView, 0, lp);

            // Hide the title template
        View* titleTemplate = mWindow->findViewById(R::id::title_template);
            titleTemplate->setVisibility(View::GONE);
    } else {
        mIconView = (ImageView*) mWindow->findViewById(R::id::icon);

        const bool hasTextTitle = mTitle.length();//!TextUtils.isEmpty(mTitle);
        if (hasTextTitle && mShowTitle) {
            // Display the title if a title is supplied, else hide it.
            mTitleView = (TextView*) mWindow->findViewById(R::id::alertTitle);
            mTitleView->setText(mTitle);

            // Do this last so that if the user has supplied any icons we
            // use them instead of the default ones. If the user has
            // specified 0 then make it disappear.
            if (mIconId.length()) {
                 mIconView->setImageResource(mIconId);
            } else if (mIcon) {
                 mIconView->setImageDrawable(mIcon);
            } else {
                // Apply the padding from the icon to ensure the title is
                // aligned correctly.
                mTitleView->setPadding(mIconView->getPaddingLeft(), mIconView->getPaddingTop(),
                            mIconView->getPaddingRight(), mIconView->getPaddingBottom());
                mIconView->setVisibility(View::GONE);
            }
        } else {
            // Hide the title template
            View* titleTemplate = mWindow->findViewById(R::id::title_template);
            titleTemplate->setVisibility(View::GONE);
            mIconView->setVisibility(View::GONE);
            topPanel->setVisibility(View::GONE);
        }
    }
}

void AlertController::setupContent(ViewGroup* contentPanel){
    mScrollView = (ScrollView*) contentPanel->findViewById(R::id::scrollView);
    mScrollView->setFocusable(false);

    // Special case for users that only want to display a String
    mMessageView = (TextView*) contentPanel->findViewById(R::id::message);
    if (mMessageView == nullptr) {
        return;
    }

    if (mMessage.length()) {
        mMessageView->setText(mMessage);
    } else {
        mMessageView->setVisibility(View::GONE);
        mScrollView->removeView(mMessageView);

        if (mListView != nullptr) {
            ViewGroup* scrollParent = (ViewGroup*) mScrollView->getParent();
            const int childIndex = scrollParent->indexOfChild(mScrollView);
            scrollParent->removeViewAt(childIndex);
            scrollParent->addView(mListView, childIndex,new LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT));
        } else {
            contentPanel->setVisibility(View::GONE);
        }
    }
}

void AlertController::onButtonClick(View&v){
    switch (v.getId()) {
     case R::id::button1://DialogInterface.BUTTON_POSITIVE:
     case R::id::button2://DialogInterface.BUTTON_NEGATIVE:
     case R::id::button3://DialogInterface.BUTTON_NEUTRAL:
         //((DialogInterface.OnClickListener) msg.obj).onClick(mDialog.get(), msg.what);
	     break;
     //case MSG_DISMISS_DIALOG://((DialogInterface) msg.obj).dismiss();
     }
}

void AlertController::setupButtons(cdroid::ViewGroup*buttonPanel){
    int BIT_BUTTON_POSITIVE = 1;
    int BIT_BUTTON_NEGATIVE = 2;
    int BIT_BUTTON_NEUTRAL = 4;
    int whichButtons = 0;
    auto mButtonHandler=std::bind(&AlertController::onButtonClick,this,std::placeholders::_1);
    mButtonPositive = (Button*) buttonPanel->findViewById(R::id::button1);
    mButtonPositive->setOnClickListener(mButtonHandler);

    if (mButtonPositiveText.empty()) {
        mButtonPositive->setVisibility(View::GONE);
    } else {
        mButtonPositive->setText(mButtonPositiveText);
        mButtonPositive->setVisibility(View::VISIBLE);
        whichButtons = whichButtons | BIT_BUTTON_POSITIVE;
    }

    mButtonNegative = (Button*) buttonPanel->findViewById(R::id::button2);
    mButtonNegative->setOnClickListener(mButtonHandler);
    if (mButtonNegativeText.empty()) {
        mButtonNegative->setVisibility(View::GONE);
    } else {
        mButtonNegative->setText(mButtonNegativeText);
        mButtonNegative->setVisibility(View::VISIBLE);
        whichButtons = whichButtons | BIT_BUTTON_NEGATIVE;
    }

    mButtonNeutral = (Button*) buttonPanel->findViewById(R::id::button3);
    mButtonNeutral->setOnClickListener(mButtonHandler);
    if (mButtonNeutralText.empty()) {
        mButtonNeutral->setVisibility(View::GONE);
    } else {
        mButtonNeutral->setText(mButtonNeutralText);
        mButtonNeutral->setVisibility(View::VISIBLE);
        whichButtons = whichButtons | BIT_BUTTON_NEUTRAL;
    }

    if (shouldCenterSingleButton(mContext)) {
        /* If we only have 1 button it should be centered on the layout and
         * expand to fill 50% of the available space.*/
        if (whichButtons == BIT_BUTTON_POSITIVE) {
            centerButton(mButtonPositive);
        } else if (whichButtons == BIT_BUTTON_NEGATIVE) {
            centerButton(mButtonNegative);
        } else if (whichButtons == BIT_BUTTON_NEUTRAL) {
            centerButton(mButtonNeutral);
        }
    }

    if (whichButtons) {
        buttonPanel->setVisibility(View::GONE);
    }
}

void AlertController::centerButton(Button* button) {
    LinearLayout::LayoutParams* params = (LinearLayout::LayoutParams*) button->getLayoutParams();
    params->gravity = Gravity::CENTER_HORIZONTAL;
    params->weight = 0.5f;
    button->setLayoutParams(params);
    View* leftSpacer = mWindow->findViewById(R::id::leftSpacer);
    if (leftSpacer) {
        leftSpacer->setVisibility(View::VISIBLE);
    }
    View* rightSpacer = mWindow->findViewById(R::id::rightSpacer);
    if (rightSpacer) {
        rightSpacer->setVisibility(View::VISIBLE);
    }
}

void AlertController::setBackground(const AttributeSet&atts,View* topPanel, View* contentPanel, View* customPanel,
    View* buttonPanel, bool hasTitle, bool hasCustomView, bool hasButtons){
}


AlertController::AlertParams::AlertParams(Context*context){
    mContext = context;
    mCancelable = true;
}

void AlertController::AlertParams::apply(AlertController* dialog){
    if (mCustomTitleView) {
        dialog->setCustomTitle(mCustomTitleView);
    } else {
        if (mTitle.length())dialog->setTitle(mTitle);
        if (mIcon) dialog->setIcon(mIcon);
        if (mIconId.length())dialog->setIcon(mIconId);
        
        if (mIconAttrId.length())
            dialog->setIcon(dialog->getIconAttributeResId(mIconAttrId));
        if (mMessage.length())dialog->setMessage(mMessage);
            
        if (mPositiveButtonText.length()) 
            dialog->setButton(DialogInterface::BUTTON_POSITIVE, mPositiveButtonText,
                    mPositiveButtonListener);
        }
        if (mNegativeButtonText.length()) {
            dialog->setButton(DialogInterface::BUTTON_NEGATIVE, mNegativeButtonText,
                    mNegativeButtonListener);
        }
        if (mNeutralButtonText.length()) {
            dialog->setButton(DialogInterface::BUTTON_NEUTRAL, mNeutralButtonText,
                    mNeutralButtonListener);
        }
        if (mForceInverseBackground) {
            dialog->setInverseBackgroundForced(true);
        }
        // For a list, the client can either supply an array of items or an
        // adapter or a cursor
        if (mItems.size()/* || mCursor*/ || mAdapter) {
            createListView(dialog);
        }
        if (mView) {
            if (mViewSpacingSpecified) {
                dialog->setView(mView, mViewSpacingLeft, mViewSpacingTop, mViewSpacingRight,
                         mViewSpacingBottom);
            } else {
                dialog->setView(mView);
            }
        } else if (mViewLayoutResId.length()) {
            dialog->setView(mViewLayoutResId);
        }
}
class AlertListAdapter:public ArrayAdapter<std::string>{
public:
    AlertListAdapter(Context*ctx,const std::string&resource,int field)
       :ArrayAdapter<std::string>::ArrayAdapter(ctx,resource,field){
    }
    View*getView(int position, View* convertView, ViewGroup* parent){
        View* view=ArrayAdapter<std::string>::getView(position, convertView, parent);
        /*if (mCheckedItems != nullptr) {
            bool isItemChecked = mCheckedItems[position];
            if (isItemChecked) {
                listView.setItemChecked(position, true);
            }
        }*/
        return view;  
    }
};
void AlertController::AlertParams::createListView(AlertController* dialog){
    ListView* listView =(ListView*)LayoutInflater::from(mContext)->inflate(dialog->mListLayout, nullptr,false);
    ListAdapter* adapter;

    if (mIsMultiChoice) {
        if (true){//mCursor == nullptr) {
            AlertListAdapter*alertadapter = new AlertListAdapter(mContext, dialog->mMultiChoiceItemLayout, R::id::text1);
            alertadapter->addAll(mItems);
            adapter=alertadapter; 
        } else {
             /*adapter = new CursorAdapter(mContext, mCursor, false) {
                 int mLabelIndex;
                 int mIsCheckedIndex;
                 {
                      Cursor cursor = getCursor();
                      mLabelIndex = cursor.getColumnIndexOrThrow(mLabelColumn);
                      mIsCheckedIndex = cursor.getColumnIndexOrThrow(mIsCheckedColumn);
                 }

                 void bindView(View view, Context context, Cursor cursor) {
                      CheckedTextView text = (CheckedTextView) view->findViewById(R::id::text1);
                      text.setText(cursor.getString(mLabelIndex));
                      listView.setItemChecked(cursor.getPosition(), cursor.getInt(mIsCheckedIndex) == 1);
                 }

                 View* newView(Context context, Cursor cursor, ViewGroup parent) {
                     return mInflater.inflate(dialog.mMultiChoiceItemLayout,parent, false);
                 }

             };*/
        }
    } else {
        /*const std::string layout=mIsSingleChoice?dialog->mSingleChoiceItemLayout:dialog->mListItemLayout

        if (mCursor) {
            adapter = new SimpleCursorAdapter(mContext, layout, mCursor,
                    new String[] { mLabelColumn }, new int[] { R::id::text1 });
        } else if (mAdapter != nullptr) {
            adapter = mAdapter;
        } else {
            adapter = new CheckedItemAdapter(mContext, layout, R::id::text1, mItems);
        }*/
    }

    //if (mOnPrepareListViewListener) mOnPrepareListViewListener->onPrepareListView(listView);

    /* Don't directly set the adapter on the ListView as we might
     * want to add a footer to the ListView later.*/
    dialog->mAdapter = adapter;
    dialog->mCheckedItem = mCheckedItem;

    if (mOnClickListener) {
        listView->setOnItemClickListener([&](AdapterView& parent, View& v, int position, long id) {
                mOnClickListener(*dialog->mDialogInterface, position);
                if (!mIsSingleChoice) {
                    dialog->mDialogInterface->dismiss();
                }
        });
    } else if (mOnCheckboxClickListener) {
        listView->setOnItemClickListener([&](AdapterView& parent, View& v, int position, long id) {
                if (mCheckedItems.size()) {
                    mCheckedItems[position] = listView->isItemChecked(position);
                }
                mOnCheckboxClickListener(*dialog->mDialogInterface, position, listView->isItemChecked(position));
        });
    }

    // Attach a given OnItemSelectedListener to the ListView
    if (mOnItemSelectedListener.onItemSelected||mOnItemSelectedListener.onNothingSelected)
        listView->setOnItemSelectedListener(mOnItemSelectedListener);

    if (mIsSingleChoice) {
        listView->setChoiceMode(ListView::CHOICE_MODE_SINGLE);
    } else if (mIsMultiChoice) {
        listView->setChoiceMode(ListView::CHOICE_MODE_MULTIPLE);
    }
    //listView->mRecycleOnMeasure = mRecycleOnMeasure;
    dialog->mListView = listView;
}

}//namespace

