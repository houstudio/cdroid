#include <cdroid.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);

    w->setTextAlignment(View::TEXT_ALIGNMENT_CENTER);
    RelativeLayout* layout=new RelativeLayout(&app);
    RelativeLayout::LayoutParams*lp;

    TextView*toptxt=new TextView(&app); toptxt->setText("1 ParentTop" );
    toptxt->setId(1);
    toptxt->setBackgroundColor(0xFF111111);
    lp=new RelativeLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT);
    lp->addRule(RelativeLayout::ALIGN_PARENT_TOP);
    layout->addView(toptxt,lp);
    
    TextView*tvpb=new TextView(&app); tvpb->setText("2 ParentBottom" );
    tvpb->setId(2);
    tvpb->setBackgroundColor(0xFF222222);
    lp=new RelativeLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT);
    lp->addRule(RelativeLayout::ALIGN_PARENT_BOTTOM);
    layout->addView(tvpb,lp);

    TextView*tvab2=new TextView(&app); tvab2->setText("Above 2" );
    tvab2->setId(3);
    tvab2->setTextAlignment(View::TEXT_ALIGNMENT_GRAVITY);
    tvab2->setGravity(Gravity::CENTER);
    tvab2->setBackgroundColor(0x80555588);
    lp=new RelativeLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT);
    lp->addRule(RelativeLayout::ABOVE,2);
    lp->addRule(RelativeLayout::RIGHT_OF,10);
    lp->addRule(RelativeLayout::LEFT_OF,20);
    lp->addRule(RelativeLayout::CENTER_HORIZONTAL);
    layout->addView(tvab2,lp);
   
    TextView*tvcent=new TextView(&app); tvcent->setText("Center Text with multi line support.\n"
    "can you find the second line?");
    tvcent->setSingleLine(false);
    tvcent->setTextSize(32);
    lp=new RelativeLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT);
    lp->addRule(RelativeLayout::CENTER_HORIZONTAL);
    lp->addRule(RelativeLayout::CENTER_VERTICAL);
    tvcent->setBackgroundColor(0xFF00FF00);
    layout->addView(tvcent,lp);
    tvcent->setId(1000);


    TextView*tvleft=new TextView(&app); tvleft->setText("ParentLeft" );
    tvleft->setId(10);
    tvleft->setBackgroundColor(0x80993333);
    lp=new RelativeLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::MATCH_PARENT);
    
    lp->addRule(RelativeLayout::BELOW,1);
    lp->addRule(RelativeLayout::ABOVE,2);
    lp->addRule(RelativeLayout::ALIGN_PARENT_LEFT);
    layout->addView(tvleft,lp);

    TextView*tvright=new TextView(&app); tvright->setText("ParentRight" );
    tvright->setId(20);
    tvright->setBackgroundColor(0x80339933);
    lp=new RelativeLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::MATCH_PARENT);
    lp->addRule(RelativeLayout::BELOW,1);
    lp->addRule(RelativeLayout::ABOVE,3);
    lp->addRule(RelativeLayout::ALIGN_PARENT_RIGHT);
    layout->addView(tvright,lp);

    w->addView(layout);
    layout->requestLayout();
    w->requestLayout();
    app.exec();
}
