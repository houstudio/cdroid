#include <cdroid.h>
#include <cdlog.h>
#include <animation/animations.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <algorithm>
class MyAdapter:public ArrayAdapter<std::string>{
private:
    int itemType;
public:
    MyAdapter(int type=0):ArrayAdapter(){
        itemType=type;
    }
    View*getView(int position, View* convertView, ViewGroup* parent)override{

        TextView*tv=(TextView*)convertView;
        if(convertView==nullptr){
            if(itemType==0) tv=new TextView("",600,20);
            else tv=new CheckBox("",600,20);
            tv->setPadding(20,0,0,0);
            tv->setSingleLine(true);
        }
        if(itemType==1)tv->setLayoutDirection(position<10?View::LAYOUT_DIRECTION_RTL:View::LAYOUT_DIRECTION_LTR);
        tv->setId(position);
        tv->setText("position :"+std::to_string(position));
        tv->setTextColor(0xFFFFFFFF);
        tv->setBackgroundColor(0x80002222);
        tv->setTextSize(24);
        return tv;
    }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);

    // Window::doLayout now always lays out direct children, so absolute layout() on
    // multiple direct children piles them up at (0,0). Put the two ListViews + toggle
    // side by side in a horizontal LinearLayout (avoids ListView-inside-ScrollView).
    LinearLayout*content=new LinearLayout(-1,-1);
    content->setOrientation(LinearLayout::HORIZONTAL);
    w->addView(content);
    auto add=[&](View*v,int ww,int hh){
        LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(ww,hh);
        lp->leftMargin=lp->topMargin=10;
        content->addView(v,lp);
    };

    MyAdapter*adapter=new MyAdapter(0);

    Animation *anim= new ScaleAnimation(0.5,1,0.1,1,Animation::RELATIVE_TO_PARENT,.5,Animation::RELATIVE_TO_SELF,.5);
    anim->setDuration(500);
    w->setId(10);
    LayoutAnimationController*lac = new LayoutAnimationController(anim,0.02);
    ListView*lv = new ListView(460,500);
    add(lv,460,500);
    lv->setId(100);
    adapter->setNotifyOnChange(true);
    lv->setAdapter(adapter);
    for(int i=0;i<56;i++) adapter->add("");

    lv->setLayoutAnimation(lac);
    adapter->notifyDataSetChanged();
    lv->startLayoutAnimation();
    lv->setVerticalScrollBarEnabled(true);
    lv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    lv->setSmoothScrollbarEnabled(true);
    lv->setSelector(new ColorDrawable(0x8800FF00));
    //lv->setSelection(2);
    lv->setDivider(new ColorDrawable(0x80224422));
    lv->setDividerHeight(1);
    lv->setOnItemClickListener([](AdapterView&lv,View&v,int pos,long id){
        LOGD("clicked %d",pos);
    });

    ListView::OnItemSelectedListener listener={nullptr,nullptr};
    listener.onItemSelected=[](AdapterView&lv,View&v,int pos,long id){
        LOGD("selected position %d",pos);
    };
    lv->setOnItemSelectedListener(listener);
////////////////////////////////////////////////////////////////////////////////////////

    MyAdapter*adapter2=new MyAdapter(1);
    ListView*lv2 = new ListView(500,500);
    add(lv2,500,500);
    lv2->setId(200);
    ToggleButton *toggle=new ToggleButton(300,40);
    add(toggle,300,40);
    lv2->setAdapter(adapter2);
    for(int i=0;i<56;i++)  adapter2->add("");

    lv2->setDivider(new ColorDrawable(0x80224422));
    lv2->setDividerHeight(1);
    lv2->setVerticalScrollBarEnabled(true);
    lv2->setSmoothScrollbarEnabled(true);
    lv2->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    lv2->setAdapter(adapter2);
    lv2->setSelector(new ColorDrawable(0x88FF0000));
    adapter2->notifyDataSetChanged();
    lv2->setOnItemClickListener([&](AdapterView&lv,View&v,int pos,long id){
        LOGD("clicked %d",pos);
    });

    // CHOICE_MODE_MULTIPLE_MODAL: 长按列表项进入多选 ActionMode (浮窗菜单),
    // 验证 startActionMode 上浮到 Window 创建 + onCreate/onPrepare/onActionItemClicked/
    // onItemCheckedStateChanged/onDestroyActionMode 回调链 + 选中数归 0 自动 finish。
    AbsListView::MultiChoiceModeListener mc;
    mc.onCreateActionMode = [](ActionMode& mode, Menu& menu) -> bool {
        menu.add(0, 1, 0, "Delete");
        menu.add(0, 2, 0, "Select All");
        return true;
    };
    mc.onPrepareActionMode = [lv2](ActionMode& mode, Menu& menu) -> bool {
        mode.setTitle(std::to_string(lv2->getCheckedItemCount()) + " selected");
        return true;
    };
    mc.onActionItemClicked = [lv2, adapter2](ActionMode& mode, MenuItem& item) -> bool {
        if (item.getItemId() == 1) {   // Delete: 删除所有选中项 (从大到小删, 避免索引移位)
            SparseBooleanArray checked;
            lv2->getCheckedItemPositions(checked);
            std::vector<int> positions;
            for (size_t i = 0; i < checked.size(); i++)
                if (checked.valueAt(i)) positions.push_back(checked.keyAt(i));
            std::sort(positions.rbegin(), positions.rend());
            for (int pos : positions) adapter2->removeAt(pos);
            adapter2->notifyDataSetChanged();
            mode.finish();
            return true;
        }
        if (item.getItemId() == 2) {   // Select All
            const int n = adapter2->getCount();
            for (int i = 0; i < n; i++) lv2->setItemChecked(i, true);
            return true;
        }
        return false;
    };
    mc.onItemCheckedStateChanged = [lv2](ActionMode& mode, int position, long id, bool checked) {
        mode.setTitle(std::to_string(lv2->getCheckedItemCount()) + " selected");
        LOGD("multichoice %d checked=%d  total=%d", position, checked, lv2->getCheckedItemCount());
    };
    mc.onDestroyActionMode = [](ActionMode&) { LOGD("== multi-choice action mode destroyed =="); };
    lv2->setMultiChoiceModeListener(mc);

    toggle->setTextOn("Modal");
    toggle->setTextOff("Single");
    toggle->setBackgroundResource("cdroid:drawable/btn_toggle_bg.xml");
    toggle->setOnCheckedChangeListener([&](CompoundButton&view,bool check){
        lv2->setChoiceMode(check ? ListView::CHOICE_MODE_MULTIPLE_MODAL : ListView::CHOICE_MODE_SINGLE);
    });
    lv2->setChoiceMode(ListView::CHOICE_MODE_MULTIPLE_MODAL);

    Runnable rd;
    rd=[&rd,lv,w,adapter](){
        for(int i=0;i<10;i++)adapter->add("");
        lv->startLayoutAnimation();
        //w->postDelayed(rd,1000);
    };
    w->postDelayed(rd,10000);
    content->requestLayout();
    app.exec();
    return 0;
};
