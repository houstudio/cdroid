/*********************************************************************************
 * ActionBar demo: setActionBar(Toolbar*) adopts a Toolbar as the Activity's
 * ActionBar. Verifies the Activity-side options-menu dispatch chain
 * (onCreateOptionsMenu / onPrepareOptionsMenu / onOptionsItemSelected / onNavigateUp)
 * is actually driven through the ToolbarActionBar bridge.
 *
 * Watch the log (LOGD) for: onCreateOptionsMenu at setActionBar time,
 * onOptionsItemSelected on menu tap, onNavigateUp on the Up affordance tap.
 *********************************************************************************/
#include <cdroid.h>
#include <widget/toolbar.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <widget/R.h>
#include <core/attributeset.h>
#include <porting/cdlog.h>

using namespace cdroid;

class ActionBarActivity : public Window {
public:
    TextView* mBody = nullptr;

    ActionBarActivity(int x,int y,int w,int h):Window(x,y,w,h){
        setText("ActionBar Demo"); // Window title; seeds the action bar title

        LinearLayout* root = new LinearLayout(0,0);
        root->setOrientation(LinearLayout::VERTICAL);
        addView(root);

        // Toolbar declared in the content layout, then adopted as the ActionBar.
        Toolbar* tb = new Toolbar(getContext(), AttributeSet(getContext(),"cdroid"));
        tb->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::WRAP_CONTENT));
        root->addView(tb);

        setActionBar(tb);                       // builds ToolbarActionBar, fires onCreateOptionsMenu
        getActionBar()->setDisplayHomeAsUpEnabled(true);
        getActionBar()->setTitle("Hello ActionBar");

        mBody = new TextView("press a menu item (or Up)", 800, 240);
        mBody->setTextSize(28);
        mBody->setBackgroundColor(0xFF223344);
        mBody->setTextColor(0xFFFFFFFF);
        mBody->setGravity(Gravity::CENTER);
        root->addView(mBody);
    }

    bool onCreateOptionsMenu(Menu& menu) override{
        LOGD("== onCreateOptionsMenu ==");
        menu.add(0, 1, 0, "Settings");
        menu.add(0, 2, 0, "About");
        menu.add(0, 3, 0, "Hide");
        return true;
    }

    bool onOptionsItemSelected(MenuItem& item) override{
        LOGD("== onOptionsItemSelected id=%d (%s) ==", item.getItemId(), item.getTitle().c_str());
        if(item.getItemId() == R::id::home){
            // Defer to the default Activity impl, which routes home -> onNavigateUp.
            return Window::onOptionsItemSelected(item);
        }
        if(item.getItemId() == 3){            // "Hide" toggles the action bar
            if(getActionBar()->isShowing()) getActionBar()->hide();
            else                              getActionBar()->show();
            return true;
        }
        mBody->setText("clicked: " + item.getTitle());
        return true;
    }

    bool onNavigateUp() override{
        LOGD("== onNavigateUp ==");
        mBody->setText("navigate up");
        return true;
    }
};

int main(int argc,const char* argv[]){
    App app(argc, argv);
    ActionBarActivity* act = new ActionBarActivity(0, 0, -1, -1);
    return app.exec();
}
