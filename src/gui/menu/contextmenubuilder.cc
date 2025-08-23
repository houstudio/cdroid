public class ContextMenuBuilder:public MenuBuilder,public ContextMenu {

    @UnsupportedAppUsage
    public ContextMenuBuilder(Context context) {
        super(context);
    }

    public ContextMenu setHeaderIcon(Drawable icon) {
        return (ContextMenu) super.setHeaderIconInt(icon);
    }

    public ContextMenu setHeaderIcon(int iconRes) {
        return (ContextMenu) super.setHeaderIconInt(iconRes);
    }

    public ContextMenu setHeaderTitle(CharSequence title) {
        return (ContextMenu) super.setHeaderTitleInt(title);
    }

    public ContextMenu setHeaderTitle(int titleRes) {
        return (ContextMenu) super.setHeaderTitleInt(titleRes);
    }

    public ContextMenu setHeaderView(View view) {
        return (ContextMenu) super.setHeaderViewInt(view);
    }

    /**
     * Shows this context menu, allowing the optional original view (and its
     * ancestors) to add items.
     *
     * @param originalView Optional, the original view that triggered the
     *        context menu.
     * @param token Optional, the window token that should be set on the context
     *        menu's window.
     * @return If the context menu was shown, the {@link MenuDialogHelper} for
     *         dismissing it. Otherwise, null.
     */
    public MenuDialogHelper showDialog(View originalView, IBinder token) {
        if (originalView != null) {
            // Let relevant views and their populate context listeners populate
            // the context menu
            originalView.createContextMenu(this);
        }

        if (getVisibleItems().size() > 0) {
            EventLog.writeEvent(50001, 1);

            MenuDialogHelper helper = new MenuDialogHelper(this);
            helper.show(token);

            return helper;
        }

        return null;
    }

    public MenuPopupHelper showPopup(Context context, View originalView, float x, float y) {
        if (originalView != null) {
            // Let relevant views and their populate context listeners populate
            // the context menu
            originalView.createContextMenu(this);
        }

        if (getVisibleItems().size() > 0) {
            EventLog.writeEvent(50001, 1);

            int location[] = new int[2];
            originalView.getLocationOnScreen(location);

            final MenuPopupHelper helper = new MenuPopupHelper(
                    context,
                    this,
                    originalView,
                    false /* overflowOnly */,
                    com.android.internal.R.attr.contextPopupMenuStyle);
            helper.show(Math.round(x), Math.round(y));
            return helper;
        }

        return null;
    }
}
