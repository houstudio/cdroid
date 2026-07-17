#ifndef __ACTIONMODE_H__
#define __ACTIONMODE_H__
#include <string>
#include <functional>
#include <core/rect.h>   // Rect (CRect<int> typedef, 不能前向声明)
namespace cdroid{
class View;          // view/view.h (前向声明; 避免与 view.h 互含成环)
class Menu;          // menu/menu.h (Callback getMenu 用)
class MenuItem;      // menu/menuitem.h
class MenuInflater;  // menu/menuinflater.h
class ActionMode {
public:
    virtual ~ActionMode()=default;
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

    virtual void setCustomView(View* view)=0;

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
    virtual Menu* getMenu()=0;   // ActionMode.java:240 (CDROID: Menu 抽象 → 返指针)

    virtual std::string getTitle()=0;

    virtual std::string getSubtitle()=0;

    /**
     * Returns the current custom view for this action mode.
     * @return The current custom view
     */
    virtual View* getCustomView()=0;

    //abstract MenuInflater getMenuInflater();
    virtual MenuInflater* getMenuInflater()=0;

    void onWindowFocusChanged(bool hasWindowFocus) {}

    bool isUiFocusable() {
        return true;
    }

    struct Callback {
        std::function<bool(ActionMode&,Menu&)> onCreateActionMode;//(ActionMode& mode, Menu& menu);

        std::function<bool(ActionMode&,Menu&)>onPrepareActionMode;//(ActionMode& mode, Menu& menu);

        std::function<bool(ActionMode&,MenuItem&)> onActionItemClicked;//(ActionMode& mode, MenuItem& item);

        std::function<void(ActionMode&)> onDestroyActionMode;//(ActionMode& mode);

        // 融合自 Callback2.onGetContentRect: FloatingActionMode 定位用(content rect,
        // 如 Editor 文本选区 rect)。为空时 FloatingActionMode 用 originatingView 屏坐标兜底。
        std::function<void(ActionMode&,View&, Rect&)> onGetContentRect;//(Rect& outRect)
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
        // 实现见 actionmode.cc (需 View/Rect 完整定义, 不能留头里——否则 actionmode.h 又得含 view.h 成环)
        virtual void onGetContentRect(ActionMode& mode, View* view, Rect& outRect);
    };
};
}/*endof namespace*/
#endif/*__ACTIONMODE_H__*/

