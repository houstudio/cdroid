/*********************************************************************************
 * Fragment options-menu demo: a FragmentActivity hosts a Toolbar (setActionBar) and
 * a Fragment that calls setHasOptionsMenu(true). Verifies the dispatch chain
 *   Activity.invalidateOptionsMenu
 *     -> ToolbarActionBar.populateOptionsMenu
 *       -> FragmentActivity.onCreateOptionsMenu
 *         -> FragmentManager.dispatchCreateOptionsMenu
 *           -> Fragment.performCreateOptionsMenu -> Fragment.onCreateOptionsMenu
 * and item selection back through Fragment.onOptionsItemSelected.
 *
 * Note: CDROID's FragmentHostCallback does not auto-invalidate on setHasOptionsMenu
 * yet, so onResume calls invalidateOptionsMenu() once the Fragment is attached.
 *********************************************************************************/
#include <cdroid.h>
#include <fragment/fragment.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragmenttransaction.h>
#include <widget/toolbar.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <core/attributeset.h>
#include <porting/cdlog.h>

using cdroid::fragment::Fragment;
using cdroid::fragment::FragmentActivity;

// A headless Fragment (no UI) that contributes toolbar menu items via the dispatch chain.
class MenuFragment : public Fragment{
public:
    void onActivityCreated(cdroid::Bundle* savedInstanceState) override{
        Fragment::onActivityCreated(savedInstanceState);
        setHasOptionsMenu(true); // declare this fragment contributes menu items
    }
    void onCreateOptionsMenu(cdroid::Menu& menu, cdroid::MenuInflater& /*inflater*/) override{
        LOGD("[frag_menu] Fragment.onCreateOptionsMenu (dispatch reached the fragment)");
        menu.add(0, 100, 0, "Frag A");
        menu.add(0, 101, 0, "Frag B");
    }
    bool onOptionsItemSelected(cdroid::MenuItem& item) override{
        LOGD("[frag_menu] Fragment.onOptionsItemSelected id=%d", item.getItemId());
        return true;
    }
};

class FragMenuActivity : public FragmentActivity{
    cdroid::TextView* mBody = nullptr;
public:
    FragMenuActivity() : FragmentActivity(0, 0, -1, -1){
        cdroid::LinearLayout* root = new cdroid::LinearLayout(0, 0);
        root->setOrientation(cdroid::LinearLayout::VERTICAL);
        addView(root);

        cdroid::Toolbar* tb = new cdroid::Toolbar(getContext(), cdroid::AttributeSet(getContext(), "cdroid"));
        tb->setLayoutParams(new cdroid::LinearLayout::LayoutParams(
                cdroid::ViewGroup::LayoutParams::MATCH_PARENT, cdroid::ViewGroup::LayoutParams::WRAP_CONTENT));
        root->addView(tb);
        setActionBar(tb);                 // Toolbar becomes the Activity's ActionBar
        getActionBar()->setTitle("Fragment Menu Demo");

        mBody = new cdroid::TextView("Fragment owns the toolbar menu (overflow)", 800, 600);
        mBody->setTextSize(28);
        mBody->setTextColor(0xFFFFFFFF);
        mBody->setBackgroundColor(0xFF102030);
        mBody->setGravity(cdroid::Gravity::CENTER);
        mBody->setLayoutParams(new cdroid::LinearLayout::LayoutParams(
                cdroid::ViewGroup::LayoutParams::MATCH_PARENT, cdroid::ViewGroup::LayoutParams::MATCH_PARENT));
        root->addView(mBody);

        // Host the headless fragment in the activity's fragment container.
        getSupportFragmentManager()->beginTransaction()
            ->replace(getFragmentContainerId(), new MenuFragment())
            .commit();
    }

    void onResume() override{
        FragmentActivity::onResume();
        // The fragment is attached by now; rebuild the menu so its items reach the toolbar.
        invalidateOptionsMenu();
    }

    bool onOptionsItemSelected(cdroid::MenuItem& item) override{
        // Let FragmentActivity dispatch to the fragment first (so Fragment.onOptionsItemSelected
        // fires), then give visual feedback for the fragment-supplied items.
        bool handled = FragmentActivity::onOptionsItemSelected(item);
        if(item.getItemId() == 100 || item.getItemId() == 101){
            mBody->setText("clicked: " + item.getTitle());
            return true;
        }
        return handled;
    }
};

int main(int argc, const char* argv[]){
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    cdroid::App app(argc, argv);
    new FragMenuActivity();
    return app.exec();
}
