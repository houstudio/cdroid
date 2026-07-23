/*********************************************************************************
 * ActionMode 接通 demo: 长按文本 → 弹出 ActionMode 菜单 (Copy/Paste/Select All/Done),
 * 验证 4 个回调 (onCreateActionMode/onPrepareActionMode/onActionItemClicked/onDestroyActionMode)
 * 被 FloatingActionMode 真正调用。
 *********************************************************************************/
#include <cdroid.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <porting/cdlog.h>

using namespace cdroid;

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window* w = new Window(0,0,-1,-1);
    LinearLayout*ll=new LinearLayout(0,0);
    w->addView(ll);
    TextView* tv = new TextView("Long-press me", 800, 240);
    tv->setTextSize(36);
    tv->setBackgroundColor(0xFF223344);
    tv->setTextColor(0xFFFFFFFF);
    tv->setGravity(Gravity::CENTER);
    ll->addView(tv);

    // ActionMode 4 事件回调 (Menu/MenuItem 由 FloatingActionMode 保证非空, 用引用)
    ActionMode::Callback cb;
    cb.onCreateActionMode = [](ActionMode& mode, Menu& menu)->bool{
        LOGD("== onCreateActionMode ==");
        menu.add(0, 1, 0, "Copy");
        menu.add(0, 2, 0, "Paste");
        menu.add(0, 3, 0, "Select All");
        menu.add(0, 9, 0, "Done");
        mode.setTitle("Context Actions");
        return true;
    };
    cb.onPrepareActionMode = [](ActionMode& mode, Menu& menu)->bool{
        LOGD("== onPrepareActionMode ==");
        return false;
    };
    cb.onActionItemClicked = [tv](ActionMode& mode, MenuItem& item)->bool{
        LOGD("== onActionItemClicked id=%d (%s) ==", item.getItemId(), item.getTitle().c_str());
        if(item.getItemId() == 9){   // Done
            mode.finish();
            return true;
        }
        tv->setText("clicked: " + item.getTitle());
        return true;
    };
    cb.onDestroyActionMode = [](ActionMode& mode){
        LOGD("== onDestroyActionMode ==");
    };

    // 长按触发 startActionMode
    tv->setOnLongClickListener([&cb](View& v)->bool{
        LOGD("== long press: startActionMode ==");
        ActionMode* mode = v.startActionMode(cb);
        LOGD("== startActionMode returned mode=%p ==", mode);
        return true;
    });
    tv->setOnClickListener([](View& v){LOGD("OnClick");});
    return app.exec();
}
