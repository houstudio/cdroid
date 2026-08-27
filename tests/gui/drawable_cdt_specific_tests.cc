/* CDROID-specific drawable coverage — the surfaces the AOSP CTS ports in
 * drawable/cts_*_test.cc do NOT reach:
 *   - real aapt-compiled 9-patches off the framework resources (cdNp chunk),
 *   - the CDROID Shape family (RectShape/RoundRectShape/OvalShape/ArcShape with
 *     gradient/stroke extensions — not AOSP ShapeDrawable),
 *   - PictureDrawable recording playback and AnimatedRotateDrawable.
 * The construction+draw smoke cases that CTS covers with assertions
 * (ColorDrawable/BitmapDrawable/TransitionDrawable/InsetDrawable/ClipDrawable/
 * RotateDrawable/LayerDrawable/LevelListDrawable/GradientDrawable) and the
 * text-XML inflate cases (retired path — binary AXML is the resource mode)
 * were dropped. */
#include <gtest/gtest.h>
#include <widget/internal_R.h>
#include <cdroid.h>
#include <drawable/drawables.h>
#include <drawable/ninepatchdrawable.h>
#include <drawable/animatedstatelistdrawable.h>
#include <core/systemclock.h>
#include <core/graphdevice.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <widget/switch.h>
#include <core/path.h>
#include <image-decoders/imagedecoder.h>
#include <guienvironment.h>
#if defined(_WIN32)||defined(_WIN64)
extern void sleep(uint32_t);
extern void usleep(uint32_t);
#endif
#define SLEEP(x) usleep((x)*1000)
using namespace Cairo;
using namespace cdroid;

/* Blits the offscreen drawing surface into the shared content area through the
   unified View pipeline (the shared App's GraphDevice composes/flips) — replaces
   the old GFXInit()/GFXCreateSurface()/GFXBlit()/GFXFlip() direct-surface path. */
class SurfaceBlitView:public View{
public:
    cdroid::RefPtr<ImageSurface>mSrc;
    SurfaceBlitView(int w,int h):View(&App::getInstance()){}
    void onDraw(Canvas&c){
        View::onDraw(c);
        if(mSrc){ c.set_source(mSrc,0,0); c.paint(); }
    }
};

class DRAWABLE_CDT:public testing::Test{
public:
    static Canvas*ctx;
    static App*rm;
    static int mScreenWidth,mScreenHeight;
    static cdroid::RefPtr<ImageSurface>sImage;
    SurfaceBlitView*mView=nullptr;
public:
    static void SetUpTestCase(){
        mScreenWidth=800; mScreenHeight=600;
        /* Offscreen Cairo target only — no GFXInit()/GFXCreateSurface(). Display
           happens via SurfaceBlitView in the shared content area. */
        auto surface=Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,mScreenWidth,mScreenHeight);
        /* Reuse the global App (built by GUIEnvironment) as the resource Context —
           App is-a App, so getDrawable()/XmlPullParser resolve against the same
           .pak the rest of the harness uses. No private App, nothing to free. */
        rm=&App::getInstance();
        ctx=new Canvas(surface);
        sImage=ImageSurface::create(Surface::Format::ARGB32,400,400);
        cdroid::RefPtr<Gradient>pat=LinearGradient::create(0,0,400,400);
        cdroid::RefPtr<Gradient>rd=RadialGradient::create(20,20,30,200,200,100);
        pat->add_color_stop_rgba(0,1,0,0,0);
        pat->add_color_stop_rgba(.25,0,1,0,.5);
        pat->add_color_stop_rgba(.5,1,0,1,1.);
        pat->add_color_stop_rgba(.75,1,1,1,.5);
        pat->add_color_stop_rgba(1.,1,1,1,0);
        rd->add_color_stop_rgba(0,.2,.2,.2,.1);
        rd->add_color_stop_rgba(1,1.,.2,.5,.5);
        cdroid::RefPtr<Cairo::Context>cr=Cairo::Context::create(sImage);
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
    static void TearDownTestCase(){
        delete ctx;
        /* rm aliases the global App — never deleted here. */
    }
    virtual void SetUp(){
        /* A fresh blit view per case — content() is cleared between cases by the
           test listener (which deletes it). */
        mView=new SurfaceBlitView(mScreenWidth,mScreenHeight);
        mView->mSrc=std::dynamic_pointer_cast<ImageSurface>(ctx->get_target());
        GUIEnvironment::content()->addView(mView);
        ctx->save();
        ctx->set_source_rgba(0,0,0,1);
        ctx->rectangle(0,0,mScreenWidth,mScreenHeight);
        ctx->fill();
    }
    void postCompose(){
        if(mView&&mView->mSrc) mView->mSrc->flush();
        if(mView) mView->invalidate();
        pumpFor(16);
    }
    virtual void TearDown(){
        ctx->restore();
        postCompose();
        pumpFor(400); // dwell on the final frame
    }
};

Canvas* DRAWABLE_CDT::ctx=nullptr;
App* DRAWABLE_CDT::rm =nullptr;
int DRAWABLE_CDT::mScreenWidth=0;
int DRAWABLE_CDT::mScreenHeight=0;
cdroid::RefPtr<ImageSurface>DRAWABLE_CDT::sImage;


// Real aapt-compiled 9-patch off the framework pak (cdNp chunk + outline).
TEST_F(DRAWABLE_CDT,ninepatch1){
    NinePatchDrawable *d = (NinePatchDrawable*)rm->getDrawable(cdroid::internal::R::drawable::btn_default_transparent_normal);
    Outline outline,outline2;
    d->setBounds(0,0,d->getIntrinsicWidth(),d->getIntrinsicHeight());
    d->getOutline(outline);
    for(int i=0, w=d->getIntrinsicWidth(),h=d->getIntrinsicHeight();w<800;w+=20,i+=2){
        ctx->set_source_rgb(.4,.4,.0);
        ctx->rectangle(200,200,w,h+i);
        ctx->fill();
        d->setBounds(200,200,w,h+i);
        d->getOutline(outline2);
        d->draw(*ctx);
        postCompose();
        usleep(5000);
    }
    delete d;
}

TEST_F(DRAWABLE_CDT,ninepatch2){
    NinePatchDrawable*d = (NinePatchDrawable*)rm->getDrawable(cdroid::internal::R::drawable::btn_default_transparent_normal);
    for(int i=0, w=d->getIntrinsicWidth(),h=d->getIntrinsicHeight();w<800;w+=20,i+=2){
        ctx->set_source_rgb(.4,.4,.0);
        ctx->rectangle(200,200,w,h+i);
        ctx->fill();
        d->setBounds(200,200,w,h+i);
        d->draw(*ctx);
        postCompose();
        usleep(5000);
    }
    delete d;
}

// hdpi-only 9-patch on the (160dpi) test display: the decode-time density
// resample (AOSP BitmapFactory.decodeResourceStream semantics) must bring the
// intrinsic size into display pixels — 78x42 hdpi content scales 240→160 to
// 52x28 — instead of the legacy raw-pixel rendering.
TEST_F(DRAWABLE_CDT,ninepatch3_density_scale){
    NinePatchDrawable*d = (NinePatchDrawable*)rm->getDrawable(cdroid::internal::R::drawable::switch_thumb_holo_dark);
    ASSERT_NE(nullptr,d);
    const DisplayMetrics& dm = rm->getResources().getDisplayMetrics();
    if (dm.densityDpi == 160) {
        EXPECT_EQ(52, d->getIntrinsicWidth());
        EXPECT_EQ(28, d->getIntrinsicHeight());
    }
    delete d;
}

// ASLD steady state (never toggled): getCurrent() must be the selected ITEM
// drawable, not a transition container — AOSP shows the transition only while
// a state change animates. Exposed by the Material switch thumb
// (switch_thumb_material_anim): off item = <nine-patch>, transitions = <animation-list>.
TEST_F(DRAWABLE_CDT, asld_steady_state_current_item){
    Drawable* d = rm->getDrawable(cdroid::internal::R::drawable::switch_thumb_material_anim);
    ASSERT_NE(nullptr, d);
    auto* asld = dynamic_cast<AnimatedStateListDrawable*>(d);
    ASSERT_NE(nullptr, asld);
    asld->setState(std::vector<int>{});  // default (unchecked) keyframe
    Drawable* current = asld->getCurrent();
    ASSERT_NE(nullptr, current);
    EXPECT_TRUE(dynamic_cast<NinePatchDrawable*>(current) != nullptr)
            << "steady-state current is " << typeid(*current).name();
    delete d;
}

// Same drawable through the REAL Switch construction path (as the original
// probe saw it): the thumb's steady-state current must still be the item
// nine-patch, not the transition's AnimationDrawable.
TEST_F(DRAWABLE_CDT, asld_steady_state_via_switch){
    Switch* sw = new Switch(rm);
    sw->setText("probe");
    sw->setChecked(false);
    const int wspec = MeasureSpec::makeMeasureSpec(600, MeasureSpec::EXACTLY);
    const int hspec = MeasureSpec::makeMeasureSpec(200, MeasureSpec::AT_MOST);
    sw->measure(wspec, hspec);
    sw->layout(0, 0, sw->getMeasuredWidth(), sw->getMeasuredHeight());
    Drawable* current = sw->getThumbDrawable()->getCurrent();
    ASSERT_NE(nullptr, current);
    EXPECT_TRUE(dynamic_cast<NinePatchDrawable*>(current) != nullptr)
            << "switch thumb steady-state current is " << typeid(*current).name();
    delete sw;
}

// PictureDrawable: RecordingSurface capture -> replay through a drawable.
TEST_F(DRAWABLE_CDT,picture){
    cdroid::RefPtr<RecordingSurface>picture= RecordingSurface::create();
    cdroid::RefPtr<Cairo::Context>ctxpic=Cairo::Context::create(picture);
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

// ---- CDROID Shape family (not AOSP ShapeDrawable) ----

TEST_F(DRAWABLE_CDT,rectshape){
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
        usleep(5000);
    }
}

TEST_F(DRAWABLE_CDT,roundrect){
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

TEST_F(DRAWABLE_CDT,roundrectshape){
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
       usleep(20000);
    }
}

TEST_F(DRAWABLE_CDT,ringshape){
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
        usleep(20000);
    }
}

TEST_F(DRAWABLE_CDT,ovalshape){
    OvalShape*ov=new OvalShape();
    ov->resize(600,500);
    ctx->set_color(0xFFFF0000);
    ov->draw(*ctx);
    ctx->fill();
}

TEST_F(DRAWABLE_CDT,arcshape){
    ArcShape*arc=new ArcShape(M_PI,1.4*M_PI);
    arc->resize(600,500);
    ctx->set_color(0xFFFF0000);
    arc->draw(*ctx);
    ctx->fill();
}

// AnimatedRotateDrawable has no local cts_* port.
TEST_F(DRAWABLE_CDT,animaterotate){
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
        usleep(10000);
    }
}

// ---- Text-XML inflate path (in-memory stream -> DrawableInflater) ----






