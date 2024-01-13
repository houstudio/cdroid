#include <cdroid.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/divideritemdecoration.h>

class MyAdapter:public RecyclerView::Adapter{//<MyAdapter.ViewHolder> {
private:
    std::vector<std::string> items;
public:
    class ViewHolder:public RecyclerView::ViewHolder {
    public:
        TextView* textView;
        ViewHolder(View* itemView):RecyclerView::ViewHolder(itemView){
            textView =(TextView*)itemView;// (itemView.findViewById(R.id.textView)
        }
    };

    MyAdapter(){}
    MyAdapter(const std::vector<std::string>& items) {
        this->items = items;
    }
    MyAdapter::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) {
        TextView* view = new TextView("",200,40);
	view->setBackgroundColor(0x80234567);
	//LayoutInflater.from(parent.getContext()).inflate(R.layout.item_layout, parent, false);
        return new ViewHolder(view);
    }

    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position)override{
	std::string item = items.at(position);
	TextView*textView = ((MyAdapter::ViewHolder&)holder).textView;
        textView->setText(item);
	textView->setId(position);
	textView->setOnClickListener([position](View&v){
	    LOGD("click item positon=%d",position);
	});
    }
    void add(const std::string&str){
        items.push_back(str);
    }
    int getItemCount()override {
        return items.size();
    }
};
int main(int argc,const char*argv[]){
   App app(argc,argv);
   Window*w=new Window(0,0,-1,-1);
   w->setBackgroundColor(0xFF112233);
   RecyclerView*rv=new RecyclerView(800,600);
   MyAdapter*adapter=new MyAdapter();
   DividerItemDecoration* decoration = new DividerItemDecoration(&app, LinearLayout::VERTICAL);
   for(int i=0;i<100;i++){
       adapter->add(std::string("string ")+std::to_string(i));
   }
   decoration->setDrawable(new ColorDrawable(0xFFFF0000));
   rv->addItemDecoration(decoration);
   rv->setAdapter(adapter);
   w->addView(rv);
   app.exec();
}
