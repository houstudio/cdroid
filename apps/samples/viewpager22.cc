#include<cdroid.h>
#include<widget/listview.h>
#include<widget/gridview.h>
#include<widget/scrollview.h>
#include<widget/spinner.h>
#include<widget/horizontalscrollview.h>
#include<widget/simplemonthview.h>
#include<widgetEx/viewpager2.h>
#include<animation/animations.h>
#include<drawables/drawables.h>
#include<cdlog.h>

class MyPagerAdapter:public RecyclerView::Adapter,RecyclerView::AdapterDataObserver{
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

    MyPagerAdapter(){
        registerAdapterDataObserver(this);
    }
    MyPagerAdapter(const std::vector<std::string>& items) {
        this->items = items;
    }
    MyPagerAdapter::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) {
        TextView* view = new TextView("",-1,-1);
        view->setBackgroundColor(0xff234567);
        //view->setGravity(Gravity::CENTER);
        view->setLayoutParams(new LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT));
        return new ViewHolder(view);
    }

    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position)override{
        std::string item = items.at(position);
        TextView*textView = ((MyPagerAdapter::ViewHolder&)holder).textView;
        textView->setText(std::string("Text ")+std::to_string(position));
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
    MyPagerAdapter*pgAdapter=new MyPagerAdapter();
    w->setId(0);
    for(int i=0;i<50;i++)pgAdapter->add(""); 
    int optionid=0;
    Rect rect;
    if(argc>1)optionid=std::atoi(argv[1]);
    w->setBackgroundColor(0xFFFF0000);
    AbsListView::OnScrollListener ons={nullptr,nullptr};
    ons.onScroll=[](AbsListView&lv,int firstVisibleItem,int visibleItemCount, int totalItemCount){
        LOGV("firstVisibleItem=%d visibleItemCount=%d totalItemCount=%d",firstVisibleItem,visibleItemCount,totalItemCount);
    };
    ons.onScrollStateChanged=[](AbsListView& view, int scrollState){
        LOGV("scrollState=%d",scrollState);
    };

    ViewPager2*pager=new ViewPager2(-1,-1);
    pager->setHorizontalFadingEdgeEnabled(true);
    pager->setFadingEdgeLength(200);

    pager->setOffscreenPageLimit(5);//must >1(This value must >=(the visible view count)+2)
    pager->setAdapter(pgAdapter);
    pager->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    pgAdapter->notifyDataSetChanged();
    pager->setCurrentItem(0);//must setcurrentitem,the default item is -1.
    w->addView(pager);
    w->requestLayout();
    app.exec();
}
