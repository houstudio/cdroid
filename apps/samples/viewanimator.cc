#include <cdroid.h>

void onClick(View&v){
   ViewGroup*root=v.getParent()->getParent();
   View*hello=root->findViewById(0x12345);
   hello->setLayerType(View::LAYER_TYPE_SOFTWARE);
   LOGD("click %d",v.getId());
   switch(v.getId()){
   case 1:hello->animate().alpha(0.2f).setDuration(2000).start();break;
   case 2:hello->animate().x(-600).setDuration(2000).start();break;
   }
}
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    LinearLayout*root = new LinearLayout(0,0);
    LinearLayout*left = new LinearLayout(0,0);
    LinearLayout*right= new LinearLayout(0,0);
    root->setOrientation(LinearLayout::HORIZONTAL);
    root->setBackgroundColor(0xFF221133);
    left->setOrientation(LinearLayout::VERTICAL);
    root->addView(left,new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::MATCH_PARENT,1)).setId(1000);
    left->setBackgroundColor(0xFF202020);
    root->addView(right,new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::MATCH_PARENT,4)).setId(2000);
    root->setWeightSum(5);
    right->setBackgroundColor(0xFF112233);
    w->addView(root).setId(100);

    Button*btn=new Button("Alpha",100,32);
    left->addView(btn).setId(1).setOnClickListener(onClick);
    btn=new Button("Translate",100,32);
    left->addView(btn).setId(2).setOnClickListener(onClick);

    btn = new Button("Hello world!",200,200);
    right->addView(btn,new LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT)).setId(0x12345);
    w->requestLayout();
    app.exec();
}
