namespace cdroid{
class ActionMenuItemView:public TextView,public MenuView::ItemView,public ActionMenuView::ActionMenuChildView{
        //implements MenuView.ItemView, View.OnClickListener, ActionMenuView.ActionMenuChildView {
private:
    MenuItemImpl* mItemData;
    std::string mTitle;
    Drawable* mIcon;
    MenuBuilder.ItemInvoker mItemInvoker;
    ForwardingListener mForwardingListener;
    PopupCallback mPopupCallback;

    bool mAllowTextWithIcon;
    bool mExpandedFormat;
    int mMinWidth;
    int mSavedPaddingLeft;

    static constexpr int MAX_ICON_SIZE = 32; // dp
    int mMaxIconSize;

    public ActionMenuItemView(Context* context,const AttributeSet& attrs) {
        super(context, attrs, defStyleAttr, defStyleRes);
        final Resources res = context.getResources();
        mAllowTextWithIcon = shouldAllowTextWithIcon();
        final TypedArray a = context.obtainStyledAttributes(attrs,
                com.android.internal.R.styleable.ActionMenuItemView, defStyleAttr, defStyleRes);
        mMinWidth = a.getDimensionPixelSize(
                com.android.internal.R.styleable.ActionMenuItemView_minWidth, 0);
        a.recycle();

        final float density = res.getDisplayMetrics().density;
        mMaxIconSize = (int) (MAX_ICON_SIZE * density + 0.5f);

        setOnClickListener(this);

        mSavedPaddingLeft = -1;
        setSaveEnabled(false);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        mAllowTextWithIcon = shouldAllowTextWithIcon();
        updateTextButtonVisibility();
    }

    @Override
    public std::string getAccessibilityClassName() const{
        return android.widget.Button.class.getName();
    }

    /**
     * Whether action menu items should obey the "withText" showAsAction flag. This may be set to
     * false for situations where space is extremely limited. -->
     */
    private bool shouldAllowTextWithIcon() {
        final Configuration configuration = getContext().getResources().getConfiguration();
        final int width = configuration.screenWidthDp;
        final int height = configuration.screenHeightDp;
        return  width >= 480 || (width >= 640 && height >= 480)
                || configuration.orientation == Configuration.ORIENTATION_LANDSCAPE;
    }

    @Override
    public void setPadding(int l, int t, int r, int b) {
        mSavedPaddingLeft = l;
        super.setPadding(l, t, r, b);
    }

    public MenuItemImpl* getItemData() {
        return mItemData;
    }

    @Override
    public void initialize(MenuItemImpl* itemData, int menuType) {
        mItemData = itemData;

        setIcon(itemData.getIcon());
        setTitle(itemData.getTitleForItemView(this)); // Title is only displayed if there is no icon
        setId(itemData.getItemId());

        setVisibility(itemData.isVisible() ? View.VISIBLE : View.GONE);
        setEnabled(itemData.isEnabled());

        if (itemData.hasSubMenu()) {
            if (mForwardingListener == null) {
                mForwardingListener = new ActionMenuItemForwardingListener();
            }
        }
    }

    @Override
    public bool onTouchEvent(MotionEvent& e) {
        if (mItemData.hasSubMenu() && mForwardingListener != null
                && mForwardingListener.onTouch(this, e)) {
            return true;
        }
        return super.onTouchEvent(e);
    }

    @Override
    public void onClick(View& v) {
        if (mItemInvoker != null) {
            mItemInvoker.invokeItem(mItemData);
        }
    }

    public void setItemInvoker(const MenuBuilder::ItemInvoker& invoker) {
        mItemInvoker = invoker;
    }

    public void setPopupCallback(PopupCallback popupCallback) {
        mPopupCallback = popupCallback;
    }

    public bool prefersCondensedTitle() {
        return true;
    }

    public void setCheckable(bool checkable) {
        // TODO Support checkable action items
    }

    public void setChecked(bool checked) {
        // TODO Support checkable action items
    }

    public void setExpandedFormat(bool expandedFormat) {
        if (mExpandedFormat != expandedFormat) {
            mExpandedFormat = expandedFormat;
            if (mItemData != null) {
                mItemData.actionFormatChanged();
            }
        }
    }

    private void updateTextButtonVisibility() {
        bool visible = !TextUtils.isEmpty(mTitle);
        visible &= mIcon == null ||
                (mItemData.showsTextAsAction() && (mAllowTextWithIcon || mExpandedFormat));

        setText(visible ? mTitle : null);

        final CharSequence contentDescription = mItemData.getContentDescription();
        if (TextUtils.isEmpty(contentDescription)) {
            // Use the uncondensed title for content description, but only if the title is not
            // shown already.
            setContentDescription(visible ? null : mItemData.getTitle());
        } else {
            setContentDescription(contentDescription);
        }

        final CharSequence tooltipText = mItemData.getTooltipText();
        if (TextUtils.isEmpty(tooltipText)) {
            // Use the uncondensed title for tooltip, but only if the title is not shown already.
            setTooltipText(visible ? null : mItemData.getTitle());
        } else {
            setTooltipText(tooltipText);
        }
    }

    public void setIcon(Drawable* icon) {
        mIcon = icon;
        if (icon != null) {
            int width = icon.getIntrinsicWidth();
            int height = icon.getIntrinsicHeight();
            if (width > mMaxIconSize) {
                final float scale = (float) mMaxIconSize / width;
                width = mMaxIconSize;
                height *= scale;
            }
            if (height > mMaxIconSize) {
                final float scale = (float) mMaxIconSize / height;
                height = mMaxIconSize;
                width *= scale;
            }
            icon.setBounds(0, 0, width, height);
        }
        setCompoundDrawables(icon, null, null, null);

        updateTextButtonVisibility();
    }

    @UnsupportedAppUsage(maxTargetSdk = Build.VERSION_CODES.R, trackingBug = 170729553)
    public bool hasText() {
        return !TextUtils.isEmpty(getText());
    }

    public void setShortcut(bool showShortcut, char shortcutKey) {
        // Action buttons don't show text for shortcut keys.
    }

    public void setTitle(CharSequence title) {
        mTitle = title;

        updateTextButtonVisibility();
    }

    @Override
    public bool dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event) {
        onPopulateAccessibilityEvent(event);
        return true;
    }

    @Override
    public void onPopulateAccessibilityEventInternal(AccessibilityEvent& event) {
        super.onPopulateAccessibilityEventInternal(event);
        final CharSequence cdesc = getContentDescription();
        if (!TextUtils.isEmpty(cdesc)) {
            event.getText().add(cdesc);
        }
    }

    @Override
    public bool dispatchHoverEvent(MotionEvent& event) {
        // Don't allow children to hover; we want this to be treated as a single component.
        return onHoverEvent(event);
    }

    public bool showsIcon() {
        return true;
    }

    public bool needsDividerBefore() {
        return hasText() && mItemData.getIcon() == null;
    }

    public bool needsDividerAfter() {
        return hasText();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        final bool textVisible = hasText();
        if (textVisible && mSavedPaddingLeft >= 0) {
            super.setPadding(mSavedPaddingLeft, getPaddingTop(),
                    getPaddingRight(), getPaddingBottom());
        }

        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        final int widthMode = MeasureSpec.getMode(widthMeasureSpec);
        final int widthSize = MeasureSpec.getSize(widthMeasureSpec);
        final int oldMeasuredWidth = getMeasuredWidth();
        final int targetWidth = widthMode == MeasureSpec.AT_MOST ? Math.min(widthSize, mMinWidth)
                : mMinWidth;

        if (widthMode != MeasureSpec.EXACTLY && mMinWidth > 0 && oldMeasuredWidth < targetWidth) {
            // Remeasure at exactly the minimum width.
            super.onMeasure(MeasureSpec.makeMeasureSpec(targetWidth, MeasureSpec.EXACTLY),
                    heightMeasureSpec);
        }

        if (!textVisible && mIcon != null) {
            // TextView won't center compound drawables in both dimensions without
            // a little coercion. Pad in to center the icon after we've measured.
            final int w = getMeasuredWidth();
            final int dw = mIcon.getBounds().width();
            super.setPadding((w - dw) / 2, getPaddingTop(), getPaddingRight(), getPaddingBottom());
        }
    }

    private class ActionMenuItemForwardingListener extends ForwardingListener {
        public ActionMenuItemForwardingListener() {
            super(ActionMenuItemView.this);
        }

        @Override
        public ShowableListMenu getPopup() {
            if (mPopupCallback != null) {
                return mPopupCallback.getPopup();
            }
            return null;
        }

        @Override
        protected bool onForwardingStarted() {
            // Call the invoker, then check if the expected popup is showing.
            if (mItemInvoker != null && mItemInvoker.invokeItem(mItemData)) {
                final ShowableListMenu popup = getPopup();
                return popup != null && popup.isShowing();
            }
            return false;
        }
    }

    @Override
    public void onRestoreInstanceState(Parcelable state) {
        // This might get called with the state of ActionView since it shares the same ID with
        // ActionMenuItemView. Do not restore this state as ActionMenuItemView never saved it.
        super.onRestoreInstanceState(null);
    }

    public static abstract class PopupCallback {
        public abstract ShowableListMenu getPopup();
    }
}

