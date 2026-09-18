#ifndef __DESKCLOCK_SEARCHMENUITEMCONTROLLER_H__
#define __DESKCLOCK_SEARCHMENUITEMCONTROLLER_H__
/*********************************************************************************
 * Port of com.android.deskclock.actionbarmenu.SearchMenuItemController and
 * NavUpMenuItemController. The androidx SearchView is not ported on cdroid, so
 * the search menu item hosts a single-line EditText action view whose text
 * changes feed the OnQueryTextListener.
 *********************************************************************************/
#include <string>

#include <widget/cdwindow.h>
#include <widget/edittext.h>

#include <actionbarmenu.h>

namespace cdroid {
namespace deskclock {
namespace actionbarmenu {

class SearchMenuItemController : public MenuItemController {
public:
    /** androidx SearchView.OnQueryTextListener (query-submit unused here). */
    typedef std::function<bool(const std::string&)> OnQueryTextChange;

private:
    class SearchTextWatcher;

    int mId;
    Context* mContext;
    std::string mQueryText;
    OnQueryTextChange mOnQueryTextChange;
    /** Borrowed TextWatcher (NoCopySpan) bridging the EditText into the listener. */
    SearchTextWatcher* mTextWatcher = nullptr;
    EditText* mSearchView = nullptr;

public:
    SearchMenuItemController(Context* context, const OnQueryTextChange& onQueryTextChange);
    ~SearchMenuItemController() override;

    int getId() const override { return mId; }
    void onCreateOptionsItem(Menu& menu) override;
    void onPrepareOptionsItem(MenuItem& item) override;
    bool onOptionsItemSelected(MenuItem& item) override;

    const std::string& getQueryText() const { return mQueryText; }
    void setQueryText(const std::string& queryText) { mQueryText = queryText; }
};

/** Sets home-as-up; the Window folds the home click into onNavigateUp. */
class NavUpMenuItemController : public MenuItemController {
private:
    Window* mActivity;

public:
    explicit NavUpMenuItemController(Window* activity) : mActivity(activity) {}

    int getId() const override { return 0; /* android.R.id.home, folded by the Window */ }
    void onCreateOptionsItem(Menu& menu) override;
    void onPrepareOptionsItem(MenuItem& item) override;
    bool onOptionsItemSelected(MenuItem& item) override { return false; }
};

} // namespace actionbarmenu
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_SEARCHMENUITEMCONTROLLER_H__
