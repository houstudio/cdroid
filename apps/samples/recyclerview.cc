#include <cdroid.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/carousellayoutmanager.h>
#include <widgetEx/recyclerview/divideritemdecoration.h>
class MyAdapter:public RecyclerView::Adapter,RecyclerView::AdapterDataObserver{//<MyAdapter.ViewHolder> {
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
    MyAdapter(const std::vector<std::string>& items) {
        this->items = items;
    }
    MyAdapter::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) {
        TextView* view = new TextView("",200,64);
        view->setBackgroundColor(0xff234567);
        view->setGravity(Gravity::CENTER);
        view->setLayoutParams(new LayoutParams(LayoutParams::MATCH_PARENT,80));//LayoutParams::WRAP_CONTENT));
        //view->setLayoutParams(new LayoutParams(120,LayoutParams::MATCH_PARENT));//LayoutParams::WRAP_CONTENT));
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
	    LOGD("click item positon=%d holder lp",position,lp);
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
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    w->setBackgroundColor(0xFF112233);
    RecyclerView*rv=new RecyclerView(800,480);
    auto ps = new LinearSnapHelper();//PagerSnapHelper();
    ps->attachToRecyclerView(rv);
    MyAdapter*adapter=new MyAdapter();
    rv->setHasFixedSize(true);
    rv->getRecycledViewPool().setMaxRecycledViews(0,64);
    //adapter->setHasStableIds(true);
    rv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    rv->setAdapter(adapter);
    DividerItemDecoration* decoration = new DividerItemDecoration(&app, LinearLayout::VERTICAL);
    for(int i=0;i<100;i++){
        adapter->add(std::string("string ")+std::to_string(i));
        //adapter->notifyItemInserted(i);//Notice:call notifyItemInserted before layout will cause memleak.
    }

    decoration->setDrawable(new ColorDrawable(0xFFFF0000));
    rv->addItemDecoration(decoration);
    w->addView(rv);
    w->requestLayout();
    Runnable r;
    r=[&](){
        adapter->remove(4);
	adapter->notifyItemRemoved(4);
    };
    w->postDelayed(r,8000);
    app.exec();
}
