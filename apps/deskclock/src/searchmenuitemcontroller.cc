#include <searchmenuitemcontroller.h>

#include <R.h>

#include <widget/actionbar.h>
#include <widget/edittext.h>
#include <menu/menu.h>
#include <text/String.h>
#include <text/textwatcher.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace actionbarmenu {

/** Borrowed TextWatcher bridging the EditText into the query listener. */
class SearchMenuItemController::SearchTextWatcher : public TextWatcher {};

SearchMenuItemController::SearchMenuItemController(Context* context,
                                                   const OnQueryTextChange& onQueryTextChange)
    : mId(R::id::menu_item_search), mContext(context),
      mOnQueryTextChange(onQueryTextChange) {
}

SearchMenuItemController::~SearchMenuItemController() {
    delete mTextWatcher;
}

void SearchMenuItemController::onCreateOptionsItem(Menu& menu) {
    MenuItem* item = menu.add(Menu::NONE, mId, Menu::NONE, std::string("search"));
    item->setShowAsAction(MenuItem::SHOW_AS_ACTION_ALWAYS);

    // androidx hosts a SearchView; cdroid hosts a single-line EditText action view.
    mSearchView = new EditText(mContext);
    mSearchView->setSingleLine(true);
    mTextWatcher = new SearchTextWatcher();
    mTextWatcher->onTextChanged = [this](CharSequence& s, int, int, int) {
        mQueryText = s.toString()->str();
        if (mOnQueryTextChange) mOnQueryTextChange(mQueryText);
    };
    mSearchView->addTextChangedListener(*mTextWatcher);
    item->setActionView(mSearchView);
}

void SearchMenuItemController::onPrepareOptionsItem(MenuItem& /*item*/) {
}

bool SearchMenuItemController::onOptionsItemSelected(MenuItem& /*item*/) {
    return false;
}

void NavUpMenuItemController::onCreateOptionsItem(Menu& /*menu*/) {
    ActionBar* actionBar = mActivity->getActionBar();
    if (actionBar != nullptr) {
        actionBar->setDisplayOptions(ActionBar::DISPLAY_HOME_AS_UP,
                                     ActionBar::DISPLAY_HOME_AS_UP | ActionBar::DISPLAY_SHOW_TITLE);
    }
}

void NavUpMenuItemController::onPrepareOptionsItem(MenuItem& /*item*/) {
}

} // namespace actionbarmenu
} // namespace deskclock
} // namespace cdroid
