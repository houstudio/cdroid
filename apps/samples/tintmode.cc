#include <cdroid.h>
#include <cdlog.h>
#include <fstream>
#include <drawables/ninepatch.h>

static const char*names[]={"CLEAR","SOURCE","OVER","IN","OUT","ATOP","DEST","DEST_OVER",
	"DEST_IN","DEST_OUT","DEST_ATOP","XOR","ADD","SATURATE"};
const int opCount=sizeof(names)/sizeof(names[0]);
static const char*images[]={
	    "cdroid:mipmap/btn_default_pressed",   "cdroid:mipmap/btn_default_selected",
	    "cdroid:mipmap/btn_default_normal","cdroid:mipmap/btn_default_normal_disable_focused",
	    "cdroid:mipmap/btn_default_normal_disable","cdroid:mipmap/fastscroll_label_left_holo_dark",
	    "cdroid:mipmap/fastscroll_label_left_holo_dark"
};
const int imageCount=sizeof(images)/sizeof(images[0]);

class TintWindow:public Window{
private:
    int mTintMode;
    int mImageIndex;
    Drawable*d;
    NinePatch *np;
public:
    TintWindow(int w,int h):Window(0,0,w,h){
	mTintMode = 0;
#if 10
	addView(new Button("Button",100,40)).setPos(0,0).setOnClickListener([this](View&){
	    mTintMode=(mTintMode+1)%opCount;
	    this->invalidate();
	    np=new NinePatch(mContext->loadImage(images[mImageIndex%imageCount]));
	    d=mContext->getDrawable(images[mImageIndex%imageCount]);
	    LOGD("images=%s",images[mImageIndex%imageCount]);
	    mImageIndex+=1;
	});
	addView(new Button("...",100,40)).setPos(0,50);
#endif
    }
    void onDraw(Canvas&canvas){
	LOGD("%d %s",mTintMode,names[mTintMode]);
	canvas.save();
	canvas.set_source_rgba(.5f,0,0,1.0);
	//canvas.set_operator(Cairo::Context::Operator::CLEAR);
	canvas.rectangle(0,0,getWidth(),getHeight());
	canvas.fill();
	canvas.restore();
#if 0
	canvas.save();
	canvas.set_source_rgb(1,0,0);
	canvas.rectangle(0,0,getWidth(),getHeight());
	canvas.fill();
	canvas.restore();

	canvas.set_source_rgb(1,1,1);
	canvas.set_operator(Cairo::Context::Operator::ATOP);
	canvas.set_line_width(30.f);
	canvas.arc(getWidth()/2,getHeight()/2,getWidth()/4,0,M_PI*2.f);
	canvas.stroke();
#else
	double a=1.f;
	canvas.save();
	canvas.set_source_rgba(1,0,0,a);
	canvas.set_line_width(30.f);
	//canvas.set_operator(Cairo::Context::Operator::IN);
	canvas.arc(200,200,60,0,M_PI*2.f);
	canvas.fill();
	canvas.restore();

	canvas.save();
	canvas.set_source_rgba(0,0,1,a);
	canvas.set_operator((Cairo::Context::Operator)mTintMode);//Cairo::Context::Operator::DEST_OVER);
	canvas.rectangle(200,200,80,80);
	canvas.fill();
	canvas.restore();

        if(np==nullptr)return;
	canvas.save();
	np->setImageSize(300,300);
	np->draw(canvas,300,100);
	for(int i=1;i<6;i++)
 	    np->drawScaledPart(RECT{1,8,5,30},RECT{20+30*i,400-10*i,5*i,100+20*i},canvas);

	for(int i=0;i<5;i++)
	   np->drawScaledPart(RECT{6,1,13,7},RECT{400,450+55*i,289,7+i*10},canvas);

	canvas.save();
	canvas.set_source(np->mImage,400,0);
	LOGD("9patch.size=%dx%d",np->mImage->get_width(),np->mImage->get_height());
	canvas.rectangle(800,100,np->mImage->get_width(),np->mImage->get_height());
	canvas.clip();
	canvas.paint();
	canvas.restore();

	d->setBounds(800,100,300,300);
	d->draw(canvas);
	canvas.restore();

	ScrollBarDrawable*sd=new ScrollBarDrawable();
	sd->setParameters(400,20,120,true);
	sd->setBounds(1200,50,40,400);
	canvas.set_source_rgb(0,0,1);
	canvas.rectangle(1200,50,40,600);
	canvas.stroke();
	sd->draw(canvas);
#endif
    }
};
int main(int argc,const char*argv[]){
    App app(argc,argv);
    cdroid::Context*ctx=&app;
    Window*w=new TintWindow(1280,720);
    return app.exec();
}
