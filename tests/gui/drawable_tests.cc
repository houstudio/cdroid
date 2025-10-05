#include <gtest/gtest.h>
#include <cdroid.h>
#include <drawables/drawables.h>
#include <fstream>
#include <sstream>
#include <core/systemclock.h>
#include <core/xmlpullparser.h>
#include <drawables/drawableinflater.h>
#include <core/path.h>
#include <image-decoders/imagedecoder.h>
#if defined(_WIN32)||defined(_WIN64)
extern void sleep(uint32_t);
extern void usleep(uint32_t);
#endif
#define SLEEP(x) usleep((x)*1000)
using namespace Cairo;
class DRAWABLE:public testing::Test{
public:
    static Canvas*ctx;
    static Assets*rm;
    static int mScreenWidth,mScreenHeight;
    static RefPtr<ImageSurface>sImage;
    static GFXHANDLE mPrimarySurface,mDrawSurface;
public:
    static void SetUpTestCase(){
        int mPitch,mFormat;
        uint8_t*mBuffer;
        GFXInit();
        GFXGetDisplaySize(0,(uint32_t*)&mScreenWidth,(uint32_t*)&mScreenHeight);
        GFXCreateSurface(0,&mPrimarySurface,mScreenWidth,mScreenHeight,mFormat,1);
        GFXCreateSurface(0,&mDrawSurface,mScreenWidth,mScreenHeight,mFormat,0);
        GFXLockSurface(mDrawSurface,(void**)&mBuffer,(uint32_t*)&mPitch);
        auto surface=Cairo::ImageSurface::create(mBuffer,Cairo::Surface::Format::ARGB32,mScreenWidth,mScreenHeight,mPitch);
        printf("mPrimarySurface=%p@%p size=%dx%dx%d\r\n",mPrimarySurface,&mBuffer,mScreenWidth,mScreenHeight,mPitch);

        rm=new Assets("cdroid.pak");
        ctx=new Canvas(surface);//GraphDevice::getInstance().createContext(800,600);
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
        GFXDestroySurface(mDrawSurface);
        GFXDestroySurface(mPrimarySurface);
    }
    virtual void SetUp(){
        ctx->save();
        ctx->set_source_rgba(0,0,0,1);
        ctx->rectangle(0,0,mScreenWidth,mScreenHeight);
        ctx->fill();
    }
    void postCompose(){
        GFXBlit(mPrimarySurface,0,0,mDrawSurface,nullptr);
        GFXFlip(mPrimarySurface);
    }
    virtual void TearDown(){
        ctx->restore();
        postCompose();
        sleep(2);
    }
    Drawable*fromStream(const std::string&content){
        int type;
        auto txtis=std::make_unique<std::istringstream>(content);
        XmlPullParser parser(rm,std::move(txtis));
        AttributeSet& attrs=parser;
        while((type=parser.next()!=XmlPullParser::START_TAG)){}
        return DrawableInflater::inflateFromXml(parser.getName(),parser,attrs);
    }
};

Canvas* DRAWABLE::ctx=nullptr;
Assets* DRAWABLE::rm =nullptr;
int DRAWABLE::mScreenWidth=0;
int DRAWABLE::mScreenHeight=0;
GFXHANDLE DRAWABLE::mPrimarySurface=nullptr;
GFXHANDLE DRAWABLE::mDrawSurface=nullptr;
RefPtr<ImageSurface>DRAWABLE::sImage;

TEST_F(DRAWABLE,parsexml){
    std::ifstream fs("styles.xml");
}

TEST_F(DRAWABLE,color){
    const char*text="<color color=\"red\"/>";
    ColorDrawable*d=(ColorDrawable*)fromStream(text);
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
    NinePatchDrawable *d = (NinePatchDrawable*)rm->getDrawable("@mipmap/btn_default_transparent_normal.9.png");
    Outline outline,outline2;
    d->setBounds(0,0,d->getIntrinsicWidth(),d->getIntrinsicHeight());
    d->getOutline(outline);
    d->setBounds(50,50,600,200);
    d->getOutline(outline2);
    d->draw(*ctx);
    delete d;
}

TEST_F(DRAWABLE,ninepatch2){
    NinePatchDrawable*d = (NinePatchDrawable*)rm->getDrawable("@mipmap/btn_default_transparent_normal.9.png");
    ctx->set_source_rgb(.4,.4,.0);
    ctx->rectangle(0,0,700,300);
    ctx->fill();
    d->setBounds(50,50,600,200);
    d->draw(*ctx);
    delete d;
}

TEST_F(DRAWABLE,picture){
    RefPtr<RecordingSurface>picture= RecordingSurface::create();
    RefPtr<Cairo::Context>ctxpic=Cairo::Context::create(picture);
    ctxpic->set_source_rgba(1,1,1,1);
    ctxpic->rectangle(0,0,400,50);
    ctxpic->fill();
    ctxpic->set_source_rgba(1,0,0,1);
    ctxpic->move_to(50,20);
    ctxpic->set_font_size(32);
    ctxpic->show_text("PictureDrawable");
    ctxpic->fill();
    ctxpic->set_source_rgba(0,1,0,.5);
    ctxpic->arc(200,25,50,0,M_PI*2.f);
    ctxpic->fill();
	
    PictureDrawable*pd=new PictureDrawable(picture);
    pd->setBounds(100,100,400,50);
    
    pd->draw(*ctx);
    delete pd;
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
    rs->setSolidColor(0xFFFF0000);
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

TEST_F(DRAWABLE,roundrect){
    cdroid::Rect rect={2,2,2,2};
    std::vector<float>out={40,30,50,30};
    std::vector<float>in={};//{30,20,30,20};
    RoundRectShape*rs=new RoundRectShape(out,rect,in);
    rs->setStrokeColor(0xFFFF0000);
    rs->setGradientColors(std::vector<uint32_t>{0xFFFF0000,0xFF00FF00});//,0xFF0000FF});
    rs->setGradientType(Shape::Gradient::SWEEP);
    rs->setGradientAngle(270);
    rs->setStrokeSize(5);
    rs->setGradientCenterX(.5f);
    rs->setGradientCenterY(.5f);
    rs->setGradientRadius(250);
    rs->resize(500,500);
    ctx->set_color(0xFF00FF00);
    rs->draw(*ctx,50,50);
    postCompose();
    sleep(10);
}

TEST_F(DRAWABLE,roundrectshape){
    cdroid::Rect rect={2,2,2,2};
    std::vector<float>out={40,30,50,30};
    std::vector<float>in={};//{30,20,30,20};
    RoundRectShape*rs=new RoundRectShape(out,rect,in);
    rs->setStrokeColor(0xFFFF0000);
    rs->setGradientColors(std::vector<uint32_t>{0xFFFF0000,0xFF00FF00});//,0xFF0000FF});
    rs->setGradientType(Shape::Gradient::RADIAL);
    rs->setGradientAngle(0);
    rs->setStrokeSize(5);
    rs->setGradientCenterX(.5f);
    rs->setGradientCenterY(.5f);
    rs->setGradientRadius(250);
    rs->resize(500,500);
    ctx->set_color(0xFF00FF00);
    ctx->translate(50,50);
    
    for(int i=0;i<8;i++){
       ctx->rectangle(0,0,500,500);
       ctx->set_source_rgb(0,0,0);
       ctx->fill();
       rs->setGradientAngle(45*i);
       rs->draw(*ctx,50,50);
       ctx->save();
       ctx->translate(250,250);
       ctx->set_line_width(5);
       ctx->rotate_degrees(45*i);
       ctx->move_to(0,0);
       ctx->line_to(200,0);
       ctx->set_source_rgb(1,1,1);
       ctx->stroke();
       ctx->restore();
       postCompose();
       sleep(1);
    }
}

TEST_F(DRAWABLE,ringshape){
    OvalShape*rs=new OvalShape();
    rs->setStrokeColor(0xFFFF0000);
    rs->setSolidColor(0xFF00FF00);
    rs->setStrokeSize(5);
    rs->setThickness(80);
    rs->setGradientType(Shape::Gradient::LINEAR);
    rs->setGradientColors(std::vector<uint32_t>{0xFFFF0000,0xFF00FF00,0xFF0000FF});
    rs->setGradientRadius(100);
    rs->setGradientCenterX(.5f);
    rs->setGradientCenterY(.5f);
    rs->resize(500,500);
    ctx->set_color(0xFF00FF00);
    ctx->translate(50,50);
    for(int i=0;i<8;i++){
       ctx->rectangle(0,0,500,500);
       ctx->set_source_rgb(0,0,0);
       ctx->fill();
       rs->setGradientAngle(45*i);
       rs->draw(*ctx,50,50);
       ctx->save();
       ctx->translate(250,250);
       ctx->set_line_width(5);
       ctx->rotate_degrees(45*i);
       ctx->move_to(0,0);
       ctx->line_to(200,0);
       ctx->set_source_rgb(1,1,1);
       ctx->stroke();
       ctx->restore();
       postCompose();
       sleep(1);
    }
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

    ShapeDrawable*sd=new ShapeDrawable();
    Shape*sp=new RectShape();
    ClipDrawable *cd2=new ClipDrawable(sd,Gravity::CENTER,ClipDrawable::HORIZONTAL);
    sd->setShape(sp);
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
    StateListDrawable*sd=dynamic_cast<StateListDrawable*>(DrawableInflater::loadDrawable(nullptr,
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
    const char*text="<shape xmlns:cdroid=\"http://schemas.android.com/apk/res/android\" shape=\"oval\" useLevel=\"true\">\
      <size cdroid:width=\"80dp\" cdroid:height=\"80dp\" /> <stroke  cdroid:width=\"20dp\" cdroid:color=\"#ffff0000\"/>\
      <gradient cdroid:angle=\"180\"  cdroid:centerX=\"0.5\" cdroid:centerY=\"0.5\" cdroid:startColor=\"#ffff0000\"\
        cdroid:centerColor=\"#ff00ff00\"  cdroid:endColor=\"#ff0000ff\"  cdroid:gradientRadius=\"200dp\" \
        cdroid:type=\"radial\"/></shape>";
    int64_t t1=SystemClock::uptimeMillis();
    Drawable*d = fromStream(text);
    d->setBounds(100,100,400,400);
    int64_t t2=SystemClock::uptimeMillis();
    d->draw(*ctx);
    ASSERT_NE((void*)nullptr,dynamic_cast<ShapeDrawable*>(d));
    printf("Usedtime=%ld\r\n",t2-t1);
}
TEST_F(DRAWABLE,inflateclip){
    const char*text="<clip><shape xmlns:cdroid=\"http://schemas.android.com/apk/res/android\" \
      cdroid:shape=\"oval\" cdroid:useLevel=\"true\">\
      <size cdroid:width=\"80dp\" cdroid:height=\"80dp\" /> <stroke  cdroid:width=\"20dp\" cdroid:color=\"#ffff0000\"/>\
      <gradient cdroid:angle=\"180\"  cdroid:centerX=\"0.5\" cdroid:centerY=\"0.5\" cdroid:startColor=\"#ffff0000\"\
        cdroid:centerColor=\"#ff00ff00\"  cdroid:endColor=\"#ff0000ff\"  cdroid:gradientRadius=\"200dp\" \
        cdroid:type=\"radial\"/></shape></clip>";
    int64_t t1=SystemClock::uptimeMillis();
    Drawable*d=fromStream(text);
    d->setBounds(100,100,400,400);
    int64_t t2=SystemClock::uptimeMillis();
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
    printf("Usedtime=%ld  clip=%p child=%p\r\n",t2-t1,dynamic_cast<ClipDrawable*>(d),((ClipDrawable*)d)->getDrawable());
}
TEST_F(DRAWABLE,inflatelayer){
   const char*text="<layer-list xmlns:cdroid=\"http://schemas.android.com/apk/res/android\"> \
        <item cdroid:id=\"123\"> <shape cdroid:shape=\"rectangle\"> <corners cdroid:radius=\"5dip\" />\
            <gradient cdroid:type=\"linear\" cdroid:startColor=\"#ff9d9e9d\" cdroid:centerColor=\"#ff5a5d5a\"\
                cdroid:centerY=\"0.75\" cdroid:endColor=\"#ff747674\" cdroid:angle=\"45\"/> </shape></item>\
        <item cdroid:id=\"456\"> <clip> <shape cdroid:shape=\"rectangle\"> <corners cdroid:radius=\"100dip\" />\
        <gradient cdroid:type=\"linear\" cdroid:startColor=\"#80ffd300\" cdroid:centerColor=\"#8000ffb6\"\
              cdroid:centerX=\"0.5\" cdroid:centerY=\"0.5\" cdroid:endColor=\"#a0ff00ff\" cdroid:angle=\"90\"/>\
            </shape> </clip> </item></layer-list>";

   Drawable*d=fromStream(text);
   d->setBounds(100,100,400,400);
   int64_t t2=SystemClock::uptimeMillis();
   LayerDrawable*ld=dynamic_cast<LayerDrawable*>(d);
   ASSERT_NE((void*)nullptr,ld);
   ASSERT_NE((void*)nullptr,ld->findDrawableByLayerId(123));
   ASSERT_NE((void*)nullptr,ld->findDrawableByLayerId(456));
   ASSERT_EQ(ld->getId(0),123);
   ASSERT_EQ(ld->getId(1),456);
   ASSERT_EQ(2,ld->getNumberOfLayers());
   ClipDrawable*cd=dynamic_cast<ClipDrawable*>(ld->findDrawableByLayerId(456));
   //cd->setGravity(Gravity::CENTER);
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
   const char*text="<selector xmlns:cdroid=\"http://schemas.android.com/apk/res/android\">\
	<item cdroid:color=\"#ffff0000\" cdroid:state_selected=\"true\"/>\
        <item cdroid:color=\"#ff00ff00\" cdroid:state_focused=\"true\"/>\
	<item cdroid:color=\"#ff0000ff\" /></selector>";
   Drawable*d =fromStream(text);
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
   const char*text="<transition xmlns:cdroid=\"http://schemas.android.com/apk/res/android\">\
	<item cdroid:color=\"#ffff0000\" cdroid:state_selected=\"true\"/>\
	<item cdroid:color=\"#ff00ff00\" cdroid:state_focused=\"true\"/>\
	</transition>";
   Drawable*d = fromStream(text);
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
   const char*text="<level-list xmlns:cdroid=\"http://schemas.android.com/apk/res/android\">\
	<item cdroid:color=\"#ffff0000\" cdroid:minLevel=\"0\" cdroid:maxLevel=\"1\"/>\
	<item cdroid:color=\"#ff00ff00\" cdroid:minLevel=\"1\" cdroid:maxLevel=\"2\"/>\
	<item cdroid:color=\"#ff0000ff\" cdroid:minLevel=\"2\" cdroid:maxLevel=\"3\"/>\
	<item cdroid:minLevel=\"3\" cdroid:maxLevel=\"4\">\
	<shape cdroid:shape=\"oval\"><solid cdroid:color=\"#ffff00ff\"/></shape>\
	</item>	</level-list>";
   Drawable*d = fromStream(text);
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
TEST_F(DRAWABLE,gradient_alpha){
    GradientDrawable*gd=new GradientDrawable();
    gd->setBounds(50,50,500,500);
    gd->setShape(1);
    for(int i=0;i<100;i++){
        gd->setStroke(i*2+8,0xFFFFFFFF,i*2,i);

        ctx->set_source_rgb(0,0,0);
        ctx->rectangle(0,0,800,600);
        ctx->fill();
        gd->setColors(std::vector<int>{(int)0xFFFF0000,(int)0xFF00FF00,(int)0xFF0000FF},{.0f,.5f,1.f});
        gd->setCornerRadius(20.f-i);
        gd->setAlpha(i*255/100);
        gd->draw(*ctx);
        ctx->get_target()->write_to_png(std::string("gradient")+std::to_string(i)+".png");
        postCompose();
        usleep(1000);
    }
}
TEST_F(DRAWABLE,gradient_rectangle){
    GradientDrawable*gd=new GradientDrawable();
    gd->setBounds(50,50,500,500);
    for(int shape=0;shape<4;shape++){
        gd->setShape(shape);
        for(int i=0;i<100;i++){
            gd->setStroke(i*2+8,0xFFFFFFFF,i*2,float(shape*2));

            ctx->set_source_rgb(0,0,0);
            ctx->rectangle(0,0,800,600);
            ctx->fill();
            gd->setOrientation((GradientDrawable::Orientation)i);
            gd->setColors(std::vector<int>{(int)0xFFFF0000,(int)0xFF00FF00,(int)0xFF0000FF},{.0f,.5f,1.f});
            gd->setSize(500-20*i,500-20*i);
            gd->setCornerRadius(20.f-i);
            gd->setAlpha(i*8);
            gd->draw(*ctx);
            ctx->get_target()->write_to_png(std::string("gradient")+std::to_string(i)+".png");
            postCompose();
            usleep(1000000);
        }
   }
}



