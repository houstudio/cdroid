#include<cdroid.h>
#include<cdlog.h>
#include <widgetEx/coorinatorlayout.h>

class YourCustomBehavior:public cdroid::CoordinatorLayout::Behavior{
public:
    YourCustomBehavior():CoordinatorLayout::Behavior{
    }

    bool onDependentViewChanged(CoordinatorLayout& parent,View& child, View& dependency) {
        // Update your child view based on changes in the dependent view
        float translationY = std::min(0, dependency.getTranslationY());
        child.setTranslationY(translationY);
        return true;
    }
    bool onStartNestedScroll(CoordinatorLayout& coordinatorLayout,View& child,View& directTargetChild,
                                       View& target, int axes, int type) {
        return dynamic_cast<ScrollView*>(&target) && (axes & View::SCROLL_AXIS_VERTICAL) != 0;
    }

    void onNestedPreScroll(CoordinatorLayout& coordinatorLayout,View& child,View& target,
                              int dx, int dy,int* consumed,int type) {
        float translationY = std::max(0, std::min(child.getHeight(), child.getTranslationY() - dy));
        child.setTranslationY(translationY);
    }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    CoordinatorLayout*cl=new CoordinatorLayout(-1,-1);
    ScrollView*scroller=new ScrollView(-1,-1);
    scroller->setSmoothScrollingEnabled(true);
    scroller->setVerticalScrollBarEnabled(true);
    scroller->setOverScrollMode(View::OVER_SCROLL_ALWAYS);

    LinearLayout*layout=new LinearLayout(-1,-1);
    layout->setOrientation(LinearLayout::VERTICAL);
    scroller->addView(layout,new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,(LayoutParams::WRAP_CONTENT)));
    layout->setId(200);

    for(int i=0;i<50;i++){
        LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,100);//(LayoutParams::WRAP_CONTENT));
        lp->setMargins(5,2,5,2);
        TextView*edit=new TextView(std::string("String")+std::to_string(i),680,200);
        edit->setTextColor(0xFFFFFFFF);
        edit->setSingleLine(true);
        edit->setInputType(EditText::TYPE_ANY);
        edit->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
        edit->setBackgroundColor(0xFF000000|((i*8)<<16)|((i*8)<<8)|(i*8));
        edit->setTextSize(40);
        edit->setFocusable(i==0);
        layout->addView(edit,lp);
    }
    cl->addView(scroller,new CoordinatorLayout::LayoutParams(
			    LayoutParams::MATCH_PARENT,
			    LayoutParams::WRAP_CONTENT)).setId(100);
    TextView*tv=new TextView("Hello world",100,32);
    cl->addView(tv,new CoorinatorLayout::LayoutParams(
			    LayoutParams::MATCH_PARENT,
			    LayoutParams::WRAP_CONTENT
			    ));
    w->addView(cl);
    w->requestLayout();
    return app.exec();
}
