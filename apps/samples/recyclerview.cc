#include <cdroid.h>
#include <widgetEx/recyclerview/recyclerview.h>
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
        TextView* view = new TextView("",200,40);
        view->setBackgroundColor(0x80234567);
        view->setLayoutParams(new LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT));
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
    RecyclerView*rv=new RecyclerView(800,600);
    MyAdapter*adapter=new MyAdapter();
    rv->getRecycledViewPool().setMaxRecycledViews(0,64);
    //adapter->setHasStableIds(true);
    rv->setAdapter(adapter);
    DividerItemDecoration* decoration = new DividerItemDecoration(&app, LinearLayout::VERTICAL);

    auto anim=rv->getItemAnimator();
    anim->setRemoveDuration(200);
    anim->setAddDuration(200);
    anim->setMoveDuration(200);
    anim->setChangeDuration(200);
    rv->getLayoutManager()->requestSimpleAnimationsInNextLayout();
    for(int i=0;i<100;i++){
        adapter->add(std::string("string ")+std::to_string(i));
        adapter->notifyItemInserted(i);
    }
    decoration->setDrawable(new ColorDrawable(0xFFFF0000));
    rv->addItemDecoration(decoration);
    w->addView(rv);
    w->requestLayout();
    app.exec();
}
