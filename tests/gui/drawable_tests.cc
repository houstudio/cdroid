#include <gtest/gtest.h>
#include <windows.h>
#include <ngl_misc.h>
#include <drawables.h>
#include <fstream>
#include <sstream>
#include <core/systemclock.h>
#define SLEEP(x) usleep((x)*1000)
class DRAWABLE:public testing::Test{
public:
    static Canvas*ctx;
    static Assets*rm;
    static RefPtr<ImageSurface>sImage;
public:
    static void SetUpTestCase(){
        GFXInit();
        ctx=GraphDevice::getInstance().createContext(800,600);
        sImage=ImageSurface::create(Surface::Format::ARGB32,400,400);
        RefPtr<Gradient>pat=LinearGradient::create(0,0,400,400);
        RefPtr<Gradient>rd=RadialGradient::create(20,20,30,200,200,100);
        pat->add_color_stop_rgba(0,1,0,0,0);
        pat->add_color_stop_rgba(.25,0,1,0,.5);
        pat->add_color_stop_rgba(.5,1,0,1,1.);
        pat->add_color_stop_rgba(.75,1,1,1,.5);
        pat->add_color_stop_rgba(1.,1,1,1,0);
        rd->add_color_stop_rgba(0,.2,.2,.2,.1);
        rd->add_color_stop_rgba(1,1.,.2,.5,.5);
        RefPtr<Cairo::Context>cr=Cairo::Context::create(sImage);
        cr->set_source(pat);
        cr->rectangle(0,0,400,400);
        cr->fill();
        cr->set_source(rd);
        cr->rectangle(0,0,400,400);
        cr->fill();
        cr->set_source_rgba(.3,1.,2,.8);
        cr->set_font_size(80);
        cr->move_to(80,200);
        cr->set_line_width(4);
        cr->show_text("Image");
        cr->stroke();
    }
    static void TearDownCase(){
        delete ctx;
    }
    virtual void SetUp(){
        ctx->save();
        ctx->set_source_rgba(0,0,0,1);
        ctx->rectangle(0,0,800,600);
        ctx->fill();
    }
    void postCompose(){
        RECT rect={0,0,800,600};
        ctx->invalidate(rect);
        ctx->blit2Device(GraphDevice::getInstance().getPrimarySurface());
    }
    virtual void TearDown(){
        ctx->restore();
        postCompose();
        sleep(1);
    }
};

Canvas* DRAWABLE::ctx=nullptr;
Assets*DRAWABLE::rm =nullptr;
RefPtr<ImageSurface>DRAWABLE::sImage;

TEST_F(DRAWABLE,parsexml){
    std::ifstream fs("styles.xml");
}

TEST_F(DRAWABLE,color){
    const char*text="<color color=\"red\"/>";
    std::istringstream is(text);
    ColorDrawable*d=(ColorDrawable*)Drawable::fromStream(nullptr,is);
    ASSERT_NE(d,(void*)nullptr);
    d->setBounds(50,50,600,400);
    d->draw(*ctx);	
    delete d;
}

TEST_F(DRAWABLE,bitmapalpha){
    BitmapDrawable*d=new BitmapDrawable(sImage);
    ASSERT_EQ(d->getBitmap().get(),(void*)sImage.get());
    d->setBounds(100,100,300,300);
    for(int alpha=255;alpha>0;alpha-=5){
        ctx->set_source_rgb(0,0,.5);
        ctx->rectangle(0,0,500,500);
        ctx->fill();
        d->setAlpha(alpha);
        d->draw(*ctx);
        postCompose();
        usleep(100*1000);
    }
}

TEST_F(DRAWABLE,ninepatch1){
    std::ifstream fs("/home/houzh/Miniwin/apps/ntvplus/assets/drawable/paopao1.9.png");
    RefPtr<ImageSurface>img=ImageSurface::create_from_stream(fs);
    ASSERT_EQ(1,(int)fs.good());
    NinePatchDrawable *d=new NinePatchDrawable(img);
    d->setBounds(50,50,600,200);
    d->draw(*ctx);
    delete d;
}

TEST_F(DRAWABLE,ninepatch2){
    std::ifstream fs("/home/houzh/Miniwin/apps/ntvplus/assets/drawable/btn_normal.9.png");//paopao1.9.png");
    RefPtr<ImageSurface>img=ImageSurface::create_from_stream(fs);
    ASSERT_EQ(1,(int)fs.good());
    ctx->set_source_rgb(.4,.4,.0);
    ctx->rectangle(0,0,700,300);
    ctx->fill();
    NinePatchDrawable *d=new NinePatchDrawable(img);
    d->setBounds(50,50,600,200);
    d->draw(*ctx);
    delete d;
}

TEST_F(DRAWABLE,transition){
    std::vector<Drawable*>ds;
    ds.push_back(new ColorDrawable(0x80FF0000));
    ds.push_back(new BitmapDrawable(sImage));
    TransitionDrawable*td=new TransitionDrawable(ds);
    td->setBounds(100,100,400,400);
    td->startTransition(5000);
    td->setCrossFadeEnabled(true);
    for(int i=0;i<=500;i++){
        ctx->set_source_rgb(1,1,1);
        ctx->rectangle(0,0,800,600);
        ctx->fill();
        td->draw(*ctx);
        postCompose();
        usleep(1000);
    }
}

TEST_F(DRAWABLE,rectshape){
    RectShape*rs=new RectShape();
    rs->resize(600,500);
    ctx->set_color(0xFFFF0000);
    for(int alpha=255;alpha>0;alpha-=5){
        ctx->set_source_rgb(0,0,0);
        ctx->rectangle(0,0,800,600);
        ctx->fill();
        rs->draw(*ctx);
        ctx->fill();
        postCompose();
        usleep(10000);
    }
}

TEST_F(DRAWABLE,roundrectshape){
    RECT rect={2,2,2,2};
    std::vector<int>out={40,30,50,30,50,30,40,30};
    std::vector<int>in={30,20,30,20,30,20,30,20};
    RoundRectShape*rs=new RoundRectShape(out,rect,in);
    rs->resize(500,500);
    ctx->set_color(0xFF00FF00);
    ctx->translate(50,50);
    rs->draw(*ctx);
    ctx->fill();
}

TEST_F(DRAWABLE,insetdrawable){
    ColorDrawable *cd=new ColorDrawable(0xFF00FF00);
    InsetDrawable*id=new InsetDrawable(cd,20);
    id->setBounds(50,50,600,400);
    ctx->set_source_rgba(1,0,0,1);
    ctx->rectangle(50,50,600,400);
    ctx->fill();
    id->draw(*ctx);
}

TEST_F(DRAWABLE,clipdrawable){
    ClipDrawable *cd=new ClipDrawable();
    BitmapDrawable*d=new BitmapDrawable(sImage);
    cd->setDrawable(d);
    cd->setBounds(50,50,400,400);

    ClipDrawable *cd2=new ClipDrawable();
    ShapeDrawable*sd=new ShapeDrawable();
    Shape*sp=new RectShape();
    cd2->setGravity(Gravity::CENTER);
    sd->setShape(sp);
    cd2->setDrawable(sd);
    sp->setSolidColor(0xFFFF0000);
    sp->setStrokeColor(0xFF00FF00);
    sp->setStrokeSize(5);
    cd2->setBounds(500,50,300,300);

    for(int i=0;i<=10000;i+=100){
       ctx->set_source_rgba(0,0,0,1);
       ctx->rectangle(0,0,800,600);
       ctx->fill();
       cd->setLevel(i);cd2->setLevel(i);
       cd->draw(*ctx);cd2->draw(*ctx);
       postCompose();
       usleep(2000);
    }
}

TEST_F(DRAWABLE,rotatedrawable){
    ColorDrawable *cd=new ColorDrawable(0xFF00FF00);
    RotateDrawable*rt=new RotateDrawable();
    rt->setDrawable(cd);
    rt->setBounds(100,100,250,250);
    rt->setPivotX(.5);
    rt->setPivotY(.5);

    RotateDrawable*rt2=new RotateDrawable();
    BitmapDrawable*bmp=new BitmapDrawable(sImage);
    rt2->setDrawable(bmp);
    rt2->setBounds(360,100,300,300);
    rt2->setPivotX(.5);
    rt2->setPivotY(.5);
    for(int i=0;i<=10000;i+=50){
        ctx->set_source_rgb(0,0,0);
        ctx->rectangle(0,0,800,600);
        ctx->fill();
	    rt->setLevel(i);
	    rt->draw(*ctx);
        rt2->setLevel(10000-i);
        rt2->draw(*ctx);
        postCompose();
        usleep(10000);
    }
}

TEST_F(DRAWABLE,layerdrawable){
    std::vector<Drawable*>cls;
    cls.push_back(new ColorDrawable(0xFFFF0000));
    cls.push_back(new ColorDrawable(0xFF00FF00));
    cls.push_back(new ColorDrawable(0xFF0000FF));
    cls.push_back(new BitmapDrawable(sImage));
    LayerDrawable*ld=new LayerDrawable(cls);
    for(int i=0;i<cls.size();i++){
        ld->setLayerGravity(i,Gravity::CENTER);
        ld->setLayerInset(i,20,20,20,20);
        ld->setLayerSize(i,400-i*20,400-i*20);
    }
    ld->setBounds(100,100,400,400);
    ld->draw(*ctx);
}
TEST_F(DRAWABLE,layerdrawable2){
    LayerDrawable*ld=dynamic_cast<LayerDrawable*>(Drawable::inflate(nullptr,"/home/houzh/Miniwin/src/gui/res/drawable/progress_horizontal.xml"));
    LayerDrawable*ld2=dynamic_cast<LayerDrawable*>(ld->getConstantState()->newDrawable());
    ld->setBounds(10,100,300,300);
    ld2->setBounds(410,100,300,300);
    ld->draw(*ctx);
    ld2->draw(*ctx);
    postCompose();
    sleep(10);
}

TEST_F(DRAWABLE,animaterotate){
    AnimatedRotateDrawable*ad=new AnimatedRotateDrawable();
    ad->setDrawable(new BitmapDrawable(sImage));
    ad->setFramesDuration(10000);
    ad->setFramesCount(100);
    ad->setBounds(100,100,500,500);
    ad->start();
    ad->setPivotX(0.5);
    ad->setPivotY(0.5);
    ad->setPivotXRelative(true);
    ad->setPivotYRelative(true);
    for(int i=0;i<100;i++){
        ctx->set_source_rgb(0,0,0);
        ctx->rectangle(0,0,800,600);
        ctx->fill();
        ad->draw(*ctx);
        ad->nextFrame();
        postCompose();
        usleep(50000);
    }
}

TEST_F(DRAWABLE,levellist){
    LevelListDrawable*ld=new LevelListDrawable();
    ld->addLevel(0,1,new ColorDrawable(0xFFFF0000));
    ld->addLevel(2,3,new ColorDrawable(0xFF00FF00));
    ld->addLevel(4,5,new ColorDrawable(0xFF0000FF));
    ld->addLevel(6,7,new ColorDrawable(0xFFFFFF00));
    ld->addLevel(8,9,new ColorDrawable(0xFF00FFFF));
    ld->addLevel(10,11,new BitmapDrawable(sImage));
    ld->setBounds(100,100,400,400);
    for(int i=0;i<=10;i++){
  	ctx->set_source_rgb(0,0,0);
        ctx->rectangle(0,0,800,600);
        ctx->fill();
        ld->setLevel(i%11);
        ld->draw(*ctx);
        postCompose();
        usleep(500000);
    }
}

TEST_F(DRAWABLE,ovalshape){
    OvalShape*ov=new OvalShape();
    ov->resize(600,500);
    ctx->set_color(0xFFFF0000);
    ov->draw(*ctx);
    ctx->fill();
}

TEST_F(DRAWABLE,arcshape){
    ArcShape*arc=new ArcShape(M_PI,1.4*M_PI);
    arc->resize(600,500);
    ctx->set_color(0xFFFF0000);
    arc->draw(*ctx);
    ctx->fill();
}
TEST_F(DRAWABLE,statelist){
    StateListDrawable*sd=dynamic_cast<StateListDrawable*>(Drawable::inflate(nullptr,
			"/home/houzh/Miniwin/src/gui/res/drawable/btn_check.xml"));
    sd->setBounds(100,100,64,64);
    std::vector<int>states={StateSet::ENABLED,StateSet::FOCUSED,StateSet::CHECKED};
    int idx=sd->getStateDrawableIndex(states);
    sd->setState(states);
    sd->draw(*ctx);
    postCompose();
    sleep(10);
}

TEST_F(DRAWABLE,inflateshape){
    const char*text="<shape shape=\"oval\" useLevel=\"true\">\
      <size width=\"80dp\" height=\"80dp\" /> <stroke  width=\"20dp\" color=\"#ffff0000\"/>\
      <gradient angle=\"180\"  centerX=\"0.5\" centerY=\"0.5\" startColor=\"#ffff0000\"\
        centerColor=\"#ff00ff00\"  endColor=\"#ff0000ff\"  gradientRadius=\"200dp\" \
        type=\"radial\"/></shape>";
    LONGLONG t1=SystemClock::uptimeMillis();
    std::istringstream is(text);
    Drawable*d=Drawable::fromStream(nullptr,is);
    d->setBounds(100,100,400,400);
    LONGLONG t2=SystemClock::uptimeMillis();
    d->draw(*ctx);
    ASSERT_NE((void*)nullptr,dynamic_cast<ShapeDrawable*>(d));
    printf("Usedtime=%lld\r\n",t2-t1);
}
TEST_F(DRAWABLE,inflateclip){
    const char*text="<clip><shape shape=\"oval\" useLevel=\"true\">\
      <size width=\"80dp\" height=\"80dp\" /> <stroke  width=\"20dp\" color=\"#ffff0000\"/>\
      <gradient angle=\"180\"  centerX=\"0.5\" centerY=\"0.5\" startColor=\"#ffff0000\"\
        centerColor=\"#ff00ff00\"  endColor=\"#ff0000ff\"  gradientRadius=\"200dp\" \
        type=\"radial\"/></shape></clip>";
    LONGLONG t1=SystemClock::uptimeMillis();
    std::istringstream is(text);
    Drawable*d=Drawable::fromStream(nullptr,is);
    d->setBounds(100,100,400,400);
    LONGLONG t2=SystemClock::uptimeMillis();
    for(int i=0;i<10000;i+=100){
	ctx->set_source_rgb(0,0,0);
	ctx->rectangle(0,0,800,600);
	ctx->fill();
        ((ClipDrawable*)d)->setLevel(i);
        d->draw(*ctx);
	postCompose();
        usleep(1000);
    }
    ASSERT_NE((void*)nullptr,dynamic_cast<ClipDrawable*>(d));
    printf("Usedtime=%lld  clip=%p child=%p\r\n",t2-t1,dynamic_cast<ClipDrawable*>(d),((ClipDrawable*)d)->getDrawable());
}
TEST_F(DRAWABLE,inflatelayer){
   const char*text="<layer-list> \
        <item id=\"123\"> <shape shape=\"rectangle\"> <corners radius=\"5dip\" />\
            <gradient type=\"linear\" startColor=\"#ff9d9e9d\"   centerColor=\"#ff5a5d5a\"\
                centerY=\"0.75\"  endColor=\"#ff747674\"  angle=\"45\"/> </shape></item>\
        <item id=\"456\"> <clip> <shape shape=\"rectangle\"> <corners radius=\"100dip\" />\
        <gradient type=\"linear\" startColor=\"#80ffd300\"   centerColor=\"#8000ffb6\"\
              centerX=\"0.5\" centerY=\"0.5\" endColor=\"#a0ff00ff\"  angle=\"90\"/>\
            </shape> </clip> </item></layer-list>";

   std::istringstream is(text);
   Drawable*d=Drawable::fromStream(nullptr,is);
   d->setBounds(100,100,400,400);
   LONGLONG t2=SystemClock::uptimeMillis();
   LayerDrawable*ld=dynamic_cast<LayerDrawable*>(d);
   ASSERT_NE((void*)nullptr,ld);
   ASSERT_NE((void*)nullptr,ld->findDrawableByLayerId(123));
   ASSERT_NE((void*)nullptr,ld->findDrawableByLayerId(456));
   ASSERT_EQ(ld->getId(0),123);
   ASSERT_EQ(ld->getId(1),456);
   ASSERT_EQ(2,ld->getNumberOfLayers());
   ClipDrawable*cd=dynamic_cast<ClipDrawable*>(ld->findDrawableByLayerId(456));
   cd->setGravity(Gravity::CENTER);
   for(int i=0;i<10000;i+=100){
       ctx->set_source_rgba(0,0,0,1);
       ctx->rectangle(0,0,800,600);
       ctx->fill();
       d->setLevel(i);
       d->draw(*ctx);
       postCompose();
       usleep(5000);
   }
}

TEST_F(DRAWABLE,inflateselector){
   const char*text="<selector>\
	<item color=\"#ffff0000\" state_selected=\"true\"/>\
        <item color=\"#ff00ff00\" state_focused=\"true\"/>\
	<item color=\"#ff0000ff\" /></selector>";
   std::istringstream is(text);
   Drawable*d=Drawable::fromStream(nullptr,is);
   d->setBounds(100,100,400,400);
   ASSERT_NE((void*)nullptr,(void*)d);
   StateListDrawable*sd=dynamic_cast<StateListDrawable*>(d);
   ASSERT_NE((void*)nullptr,(void*)sd);
   ASSERT_EQ(3,sd->getStateCount());
   ASSERT_NE((void*)nullptr,sd->getStateDrawable(0));
   ColorDrawable*cd=dynamic_cast<ColorDrawable*>(sd->getStateDrawable(0));
   ASSERT_EQ(0xFFFF0000,(unsigned int)cd->getColor());
}
TEST_F(DRAWABLE,inflatetransition){
   const char*text="<transition>\
	<item color=\"#ffff0000\" state_selected=\"true\"/>\
	<item color=\"#ff00ff00\" state_focused=\"true\"/>\
	</transition>";
   std::istringstream is(text);
   Drawable*d=Drawable::fromStream(nullptr,is);
   d->setBounds(100,100,400,400);
   ASSERT_NE((void*)nullptr,(void*)d);
   TransitionDrawable*td=dynamic_cast<TransitionDrawable*>(d);
   ASSERT_NE((void*)nullptr,(void*)td);
   ASSERT_EQ(2,td->getNumberOfLayers());
   ASSERT_NE((void*)nullptr,td->getDrawable(0));

   ColorDrawable*cd=dynamic_cast<ColorDrawable*>(td->getDrawable(0));
   ASSERT_EQ(0xFFFF0000,(unsigned int)cd->getColor());
   cd =dynamic_cast<ColorDrawable*>(td->getDrawable(1));
   ASSERT_EQ(0xFF00FF00,(unsigned int)cd->getColor());
   td->startTransition(5000);
   td->setCrossFadeEnabled(true);
   for(int i=0;i<10000;i+=100){
       ctx->set_source_rgba(0,0,0,1);
       ctx->rectangle(0,0,800,600);
       ctx->fill();
       d->setLevel(i);
       d->draw(*ctx);
       postCompose();
       usleep(50000);
   }
}

TEST_F(DRAWABLE,inflatelevellist){
   const char*text="<level-list>\
	<item color=\"#ffff0000\" minLevel=\"0\" maxLevel=\"1\"/>\
	<item color=\"#ff00ff00\" minLevel=\"1\" maxLevel=\"2\"/>\
	<item color=\"#ff0000ff\" minLevel=\"2\" maxLevel=\"3\"/>\
	<item minLevel=\"3\" maxLevel=\"4\">\
	<shape shape=\"oval\"><solid color=\"#ffff00ff\"/></shape>\
	</item>	</level-list>";
   std::istringstream is(text);
   Drawable*d=Drawable::fromStream(nullptr,is);
   d->setBounds(100,100,500,500);
   LevelListDrawable*ld=dynamic_cast<LevelListDrawable*>(d);
   ASSERT_NE((void*)nullptr,ld->getChild(0));
   ColorDrawable*cd=dynamic_cast<ColorDrawable*>(ld->getChild(0));
   ASSERT_EQ(0xffff0000,(unsigned int)cd->getColor());
    for(int i=0;i<100;i++){
       ctx->set_source_rgba(0,0,0,1);
       ctx->rectangle(0,0,800,600);
       ctx->fill();
       d->setLevel(i%5);
       d->draw(*ctx);
       postCompose();
       usleep(500000);
   }
}
