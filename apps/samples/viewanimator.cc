#include <cdroid.h>

void onClick(View&v){
   ViewGroup*root=v.getParent()->getParent();
   View*hello=root->findViewById(0x12345);
   hello->setLayerType(View::LAYER_TYPE_SOFTWARE);
   LOGD("click %d tag=%p",v.getId(),v.getTag());
   int* tag=(int*)v.getTag();
   switch(v.getId()){
   case 1:hello->animate().alpha(tag==0?0.2f:1.f).setDuration(1000).start();break;
   case 2:hello->animate().x((tag==0)?-600:0).setDuration(1000).start();break;
   }
   v.setTag((tag==0)?(void*)1:(void*)0);
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
    root->addView(left,new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::MATCH_PARENT,1));
    left->setId(1000);
    left->setBackgroundColor(0xFF202020);
    root->addView(right,new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::MATCH_PARENT,4));
    right->setId(2000);
    root->setWeightSum(5);
    right->setBackgroundColor(0xFF112233);
    w->addView(root);root->setId(100);

    Button*btn=new Button("Alpha",100,32);
    left->addView(btn);btn->setId(1);
    btn->setOnClickListener(onClick);
    btn=new Button("Translate",100,32);
    left->addView(btn);btn->setId(2);
    btn->setOnClickListener(onClick);

    btn = new Button("Hello world!",200,200);
    right->addView(btn,new LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT));
    btn->setId(0x12345);
    w->requestLayout();
    app.exec();
}
