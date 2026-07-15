#ifndef __ACTIONMODE_H__
#define __ACTIONMODE_H__
namespace cdroid{
class ActionMode {
public:
    static constexpr int TYPE_PRIMARY = 0;
    static constexpr int TYPE_FLOATING = 1;
    static constexpr int DEFAULT_HIDE_DURATION = -1;
private:
    void* mTag;
    bool mTitleOptionalHint;
    int mType = TYPE_PRIMARY;
public:
    void setTag(void* tag) {
        mTag = tag;
    }

    void* getTag() {
        return mTag;
    }

    virtual void setTitle(const std::string& title)=0;

    //abstract void setTitle(@StringRes int resId);

    virtual void setSubtitle(const std::string& subtitle)=0;

    //virtual void setSubtitle(@StringRes int resId)=0;

    void setTitleOptionalHint(bool titleOptional) {
        mTitleOptionalHint = titleOptional;
    }

    bool getTitleOptionalHint() const{
        return mTitleOptionalHint;
    }

    bool isTitleOptional() const{
        return false;
    }

    virtual void setCustomView(View* view);

    void setType(int type) {
        mType = type;
    }

    int getType() const{
        return mType;
    }

    virtual void invalidate()=0;

    void invalidateContentRect() {}

    void hide(long duration) {}

    virtual void finish()=0;

    //virtual Menu getMenu()=0;

    virtual std::string getTitle()=0;

    virtual std::string getSubtitle()=0;

    /**
     * Returns the current custom view for this action mode.
     * @return The current custom view
     */
    virtual View* getCustomView()=0;

    //abstract MenuInflater getMenuInflater();

    void onWindowFocusChanged(bool hasWindowFocus) {}

    bool isUiFocusable() {
        return true;
    }

    struct Callback {
        /**
         * Called when action mode is first created. The menu supplied will be used to
         * generate action buttons for the action mode.
         *
         * @param mode ActionMode being created
         * @param menu Menu used to populate action buttons
         * @return true if the action mode should be created, false if entering this
         *              mode should be aborted.
         */
        bool onCreateActionMode(ActionMode& mode, Menu& menu);

        /**
         * Called to refresh an action mode's action menu whenever it is invalidated.
         *
         * @param mode ActionMode being prepared
         * @param menu Menu used to populate action buttons
         * @return true if the menu or action mode was updated, false otherwise.
         */
        bool onPrepareActionMode(ActionMode& mode, Menu& menu);

        /**
         * Called to report a user click on an action button.
         *
         * @param mode The current ActionMode
         * @param item The item that was clicked
         * @return true if this callback handled the event, false if the standard MenuItem
         *          invocation should continue.
         */
        bool onActionItemClicked(ActionMode& mode, MenuItem& item);

        /**
         * Called when an action mode is about to be exited and destroyed.
         *
         * @param mode The current ActionMode being destroyed
         */
        void onDestroyActionMode(ActionMode& mode);
    };

    /**
     * Extension of {@link ActionMode.Callback} to provide content rect information. This is
     * required for ActionModes with dynamic positioning such as the ones with type
     * {@link ActionMode#TYPE_FLOATING} to ensure the positioning doesn't obscure app content. If
     * an app fails to provide a subclass of this class, a default implementation will be used.
     */
    class Callback2:public ActionMode::Callback {

        /**
         * Called when an ActionMode needs to be positioned on screen, potentially occluding view
         * content. Note this may be called on a per-frame basis.
         *
         * @param mode The ActionMode that requires positioning.
         * @param view The View that originated the ActionMode, in whose coordinates the Rect should
         *          be provided.
         * @param outRect The Rect to be populated with the content position. Use this to specify
         *          where the content in your app lives within the given view. This will be used
         *          to avoid occluding the given content Rect with the created ActionMode.
         */
        void onGetContentRect(ActionMode& mode, View* view, Rect& outRect) {
            if (view != nullptr) {
                outRect.set(0, 0, view->getWidth(), view->getHeight());
            } else {
                outRect.set(0, 0, 0, 0);
            }
        }
    };
};
}/*endof namespace*/
#endif/*__ACTIONMODE_H__*/

