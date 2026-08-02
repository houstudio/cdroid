/*********************************************************************************
 * Context menu demo: registerForContextMenu + onCreateContextMenu + onContextItemSelected.
 * Long-press the text -> Activity.showContextMenuForChild builds a ContextMenuBuilder
 * (MenuDialogHelper shows it as a dialog) -> onCreateContextMenu fills items -> selecting
 * one routes back through onContextItemSelected.
 *********************************************************************************/
#include <cdroid.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include <menu/contextmenu.h>
#include <menu/menuitem.h>
#include <porting/cdlog.h>

using namespace cdroid;

class CtxMenuActivity : public Window {
    TextView* mBody = nullptr;
public:
    CtxMenuActivity() : Window(0, 0, -1, -1){
        LinearLayout* root = new LinearLayout(0, 0);
        root->setOrientation(LinearLayout::VERTICAL);
        addView(root);

        TextView* tv = new TextView("Long-press me", 800, 600);
        tv->setTextSize(32);
        tv->setBackgroundColor(0xFF223344);
        tv->setTextColor(0xFFFFFFFF);
        tv->setGravity(Gravity::CENTER);
        root->addView(tv);

        registerForContextMenu(tv); // long-press tv triggers the context menu

        mBody = new TextView("context item shows here", 800, 200);
        mBody->setTextSize(24);
        mBody->setGravity(Gravity::CENTER);
        root->addView(mBody);
    }

    void onCreateContextMenu(ContextMenu& menu, View& /*v*/, ContextMenuInfo* /*menuInfo*/) override{
        LOGD("[ctx_menu] onCreateContextMenu");
        menu.add(0, 1, 0, "Copy");
        menu.add(0, 2, 0, "Paste");
        menu.add(0, 3, 0, "Select All");
    }

    bool onContextItemSelected(MenuItem& item) override{
        LOGD("[ctx_menu] onContextItemSelected id=%d (%s)", item.getItemId(), item.getTitle().c_str());
        mBody->setText("context: " + item.getTitle());
        return true;
    }
};

int main(int argc, const char* argv[]){
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    App app(argc, argv);
    new CtxMenuActivity();
    return app.exec();
}
