#ifndef __RV_ADAPTER_H__
#define __RV_ADAPTER_H__
#include <widgetEx/recyclerview/recyclerview.h>
class RVAdapter:public cdroid::RecyclerView::Adapter,
                public cdroid::RecyclerView::AdapterDataObserver{
private:
    std::vector<std::string> items;
    RecyclerView*mRV;
public:
    class ViewHolder:public cdroid::RecyclerView::ViewHolder {
    public:
        TextView* textView;
        ViewHolder(View* itemView):RecyclerView::ViewHolder(itemView){
            textView =(TextView*)itemView;// (itemView.findViewById(R.id.textView)
        }
    };

    RVAdapter(){
        registerAdapterDataObserver(this);
        for(int i=0;i<100;i++){
            items.push_back("String:"+std::to_string(i));
        }
    }
    RVAdapter(const std::vector<std::string>& items){
        registerAdapterDataObserver(this);
        this->items = items;
    }
    cdroid::RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) override{
        TextView* view = new TextView("",200,64);
        view->setBackgroundColor(0xff234567);
        view->setGravity(Gravity::CENTER);
        view->setFocusableInTouchMode(true);
        view->setLayoutParams(new LayoutParams(200,LayoutParams::MATCH_PARENT));
        //view->setLayoutParams(new LayoutParams(640,LayoutParams::MATCH_PARENT));
        return new ViewHolder(view);
    }

    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position)override{
        std::string item = items.at(position);
        TextView*textView = ((RVAdapter::ViewHolder&)holder).textView;
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
        notifyItemRemoved(idx);
    }
    void add(const std::string&str){
        items.push_back(str);
        notifyItemInserted(items.size()-1);
    }
    int getItemCount()override {
        return items.size();
    }
    void setData(const std::vector<std::string>&itms){
        items =itms;
        notifyDataSetChanged();
    }
    long getItemId(int position)override {return position;}
    void onItemRangeChanged(int positionStart, int itemCount, Object* payload)override{
        LOGV("positionStart=%d itemCount=%d",positionStart,itemCount);
    }
    void onItemRangeInserted(int positionStart, int itemCount)override{
        LOGV("positionStart=%d itemCount=%d",positionStart,itemCount);
    }
};
#endif
