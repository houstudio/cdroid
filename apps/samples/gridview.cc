#include<cdroid.h>
#include<widget/listview.h>
#include<widget/gridview.h>
#include<widget/scrollview.h>
#include<widget/spinner.h>
#include<widget/horizontalscrollview.h>
#include<widget/simplemonthview.h>
#include<widget/viewpager.h>
#include<animation/animations.h>
#include<cdlog.h>
using namespace Cairo;
class MyAdapter:public ArrayAdapter<std::string>{
public:
   MyAdapter():ArrayAdapter(){
   }
   View*getView(int position, View* convertView, ViewGroup* parent)override{

       ImageView*tv=(ImageView*)convertView;
       Picture picture=nullptr;
       RefPtr<Cairo::Context>ctx=nullptr;
       PictureDrawable*pd=nullptr;
       if(convertView==nullptr){
           tv=new ImageView(600,20);
	   tv->setPadding(20,0,0,0);
           picture=Cairo::RecordingSurface::create();
           ctx=Cairo::Context::create(picture);
           PictureDrawable*pd=new PictureDrawable(picture);
           convertView=tv;
           ctx->set_source_rgb((float)(position&0xFF)/255.f,(float)(position*position>>8)/255.f,0);
           ctx->arc(((GridView*)parent)->getColumnWidth()/2,10,10,0,M_PI*2.f);
           ctx->fill();
           tv->setScaleType(FIT_XY);
           tv->setImageDrawable(pd);
       }else{
           pd=(PictureDrawable*)tv->getDrawable();
           picture=pd->getPicture();
           ctx=Cairo::Context::create(picture);
           ctx->set_source_rgb((float)(position&0xFF)/255.f,(float)(position*position>>8)/255.f,0);
           ctx->arc(((GridView*)parent)->getColumnWidth()/2,10,10,0,M_PI*2.f);
           ctx->fill();
           tv->setScaleType(FIT_XY);
       }
       tv->setId(position);

       /*tv->setText("position :"+std::to_string(position));
       tv->setTextColor(0xFFFFFFFF);
       tv->setBackgroundColor(0x80002222);
       tv->setTextSize(20);*/
       return tv;
   }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    int columns=4;
    Window*w=new Window(0,0,1280,720);
    MyAdapter* adapter=new MyAdapter();
    w->setId(0);
    for(int i=0;i<4000;i++)adapter->add(""); 
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
        
    GridView*gv=new GridView(800,640);
    w->addView(gv);
    gv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    gv->setVerticalScrollBarEnabled(true);
        
    gv->setAdapter(adapter);
    adapter->notifyDataSetChanged();

    gv->setNumColumns(columns);
    gv->setColumnWidth(300);
    gv->setHorizontalSpacing(1);
    gv->setVerticalSpacing(2);
    gv->setSelector(new ColorDrawable(0x8800FF00));
    gv->setSelection(0);
    w->requestLayout();
    Button *btn=new Button("Columns[2]",200,40);
    w->addView(btn);
    btn->layout(100,645,200,40);
    btn->setFocusable(true);
    btn->requestFocus();
    btn->setOnClickListener([&](View&v){
        columns*=2;
        gv->setNumColumns(columns);
        LOGD("setNumColumns:%d",columns);
        btn->setText(std::string("Columns[")+std::to_string(columns)+"]");
    });
    app.exec();
}
