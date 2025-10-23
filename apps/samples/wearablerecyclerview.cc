#include <cdroid.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/carousellayoutmanager.h>
#include <widgetEx/recyclerview/divideritemdecoration.h>
#include <widgetEx/recyclerview/itemtouchhelper.h>
#include <widgetEx/wear/wearablelinearlayoutmanager.h>
#include <widgetEx/wear/wearablerecyclerview.h>
class MyAdapter:public RecyclerView::Adapter,
                public RecyclerView::AdapterDataObserver{
private:
    std::vector<std::string> items;
    RecyclerView*mRV;
public:
    class ViewHolder:public RecyclerView::ViewHolder {
    public:
        TextView* textView;
        ViewHolder(View* itemView):RecyclerView::ViewHolder(itemView){
            textView =(TextView*)itemView;// (itemView.findViewById(R.id.textView)
        }
    };

    MyAdapter(){
        registerAdapterDataObserver(this);
    }
    MyAdapter(const std::vector<std::string>& items):MyAdapter(){
        this->items = items;
    }
    MyAdapter::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) {
        TextView* view = new TextView("",200,64);
        view->setBackgroundColor(0xff234567);
        view->setGravity(Gravity::CENTER);
        view->setFocusableInTouchMode(true);
        view->setLayoutParams(new LayoutParams(LayoutParams::MATCH_PARENT,60));//LayoutParams::MATCH_PARENT))
        //view->setLayoutParams(new LayoutParams(640,LayoutParams::MATCH_PARENT));
        return new ViewHolder(view);
    }

    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position)override{
	    std::string item = items.at(position);
	    TextView*textView = ((MyAdapter::ViewHolder&)holder).textView;
        textView->setText(item);
	    textView->setId(position);
	    textView->setOnClickListener([position](View&v){
	        RecyclerView*rv =  (RecyclerView*)v.getParent();
	        RecyclerView::LayoutManager*mgr = rv->getLayoutManager();
	        RecyclerView::LayoutParams*lp=(RecyclerView::LayoutParams*)v.getLayoutParams();
	        int pos= mgr->getPosition(&v);
	        LOGD("click item positon=%d holder lp=%p",position,lp);
	    });
    }
    void remove(int idx){
        items.erase(items.begin()+idx);
    }
    void add(const std::string&str){
        items.push_back(str);
    }
    int getItemCount()override {
        return items.size();
    }
    long getItemId(int position)override {return position;}
    void onItemRangeChanged(int positionStart, int itemCount, Object* payload)override{
        LOGV("positionStart=%d itemCount=%d",positionStart,itemCount);
    }
    void onItemRangeInserted(int positionStart, int itemCount)override{
        LOGV("positionStart=%d itemCount=%d",positionStart,itemCount);
    }
};

class SimpleCallback: public ItemTouchHelper::SimpleCallback{
private:
    MyAdapter*mAdapter;
public:
    SimpleCallback(MyAdapter*adapter):ItemTouchHelper::SimpleCallback(
        ItemTouchHelper::LEFT | ItemTouchHelper::RIGHT|ItemTouchHelper::UP|ItemTouchHelper::DOWN,
        ItemTouchHelper::LEFT | ItemTouchHelper::RIGHT|ItemTouchHelper::UP|ItemTouchHelper::DOWN){
        mAdapter = adapter;
    }
    bool onMove(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,RecyclerView::ViewHolder& target)override{
        const int fromPosition=viewHolder.getAbsoluteAdapterPosition();
        const int toPosition=target.getAbsoluteAdapterPosition();
        mAdapter->notifyItemMoved(fromPosition, toPosition);
        return true;
    }

    void onSwiped(RecyclerView::ViewHolder& viewHolder, int direction)override{
        const int swipedPosition = viewHolder.getAbsoluteAdapterPosition();
        LOGD("direction=%d swipedPosition=%d",direction,swipedPosition);
        mAdapter->remove(swipedPosition);
        mAdapter->notifyItemRemoved(swipedPosition);
    }
    int interpolateOutOfBoundsScroll(RecyclerView& recyclerView, int viewSize, 
           int viewSizeOutOfBounds, int totalSize, int64_t msSinceStartScroll) override{
        const int maxScroll = recyclerView.getHeight() / 2;
        const float decay = 0.6f;
        const int delta = (int) (viewSizeOutOfBounds * decay);
        if (delta < 0) {
            return std::max(delta, -maxScroll);
        } else {
            return std::min(delta, maxScroll);
        }
    }
};
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    w->setBackgroundColor(0xFF112233);
    RecyclerView*rv=new WearableRecyclerView(800,480);
    rv->setLayoutManager(new WearableLinearLayoutManager(&app));//new GridLayoutManager(&app,argc));
    rv->getLayoutManager()->setItemPrefetchEnabled(true);
    auto ps = new LinearSnapHelper();//PagerSnapHelper();
    ps->attachToRecyclerView(rv);
    MyAdapter*adapter = new MyAdapter();
    SimpleCallback*cbk = new SimpleCallback(adapter);
    ItemTouchHelper*touchhelper=new ItemTouchHelper(cbk);
    touchhelper->attachToRecyclerView(rv);
    rv->getRecycledViewPool().setMaxRecycledViews(0,64);
    rv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    rv->setAdapter(adapter);
    DividerItemDecoration* decoration = new DividerItemDecoration(&app, LinearLayout::VERTICAL);

    for(int i=0;i<100;i++){
        adapter->add(std::string("string ")+std::to_string(i));
    }

    decoration->setDrawable(new ColorDrawable(0xFFFF0000));
    rv->addItemDecoration(decoration);
    w->addView(rv);
    app.exec();
}
