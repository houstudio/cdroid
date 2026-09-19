#include <gtest/gtest.h>
#include <core/app.h>
#include <core/canvas.h>
#include <core/graphdevice.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <guienvironment.h>
#include <cdlog.h>
#include <cmath>
#include <cstring>
#include <string>
#include <functional>
using namespace Cairo;
using namespace cdroid;

/* A View whose onDraw delegates to a function. Each case expresses its Cairo
   drawing inline and it is rendered through the unified View pipeline (the
   shared App's GraphDevice composes/flips), instead of a private GFXInit() +
   offscreen Canvas + manual GFXFlip()/sleep(). */
class DrawView:public View{
public:
    std::function<void(Canvas&)> draw;
    DrawView(int w,int h,std::function<void(Canvas&)>d):View(&App::getInstance()),draw(std::move(d)){}
    void onDraw(Canvas&c){
        View::onDraw(c);
        if(draw) draw(c);
    }
};

/* A DrawView sized to fill the shared content area (fallback 1280x720 before the
   first layout pass). Drawing coordinates are the same absolute ones the cases
   always used. */
/* cairo save/restore with RAII: a case's clip/transform state must not leak
   into the next case's draws (the draw pipeline reuses the canvas), and a
   bare restore() is skipped when a case throws mid-draw - cairo refuses
   copy_clip_rectangle_list on a curved clip (CLIP_NOT_REPRESENTABLE), and
   that throw used to leave the shared canvas clipped, hanging Clip3. */
struct CanvasStateGuard{
    Canvas& c;
    explicit CanvasStateGuard(Canvas&ctx):c(ctx){ c.save(); }
    ~CanvasStateGuard(){ c.restore(); }
};

/* PNG fixtures live beside this source file; resolve from __FILE__ so the
   cases work regardless of the binary's working directory (light.png and
   im_game.png were never in the tree - tests used to load them from the CWD
   and failed with "file not found"). */
static cdroid::RefPtr<ImageSurface> loadFixturePng(const std::string& name){
    std::string path = __FILE__;
    const auto slash = path.find_last_of('/');
    return ImageSurface::create_from_png(path.substr(0, slash + 1) + name);
}

static DrawView* newDrawView(std::function<void(Canvas&)>d){
    ViewGroup*content=GUIEnvironment::content();
    int w = content ? content->getWidth()  : 0; if(w<=0) w=1280;
    int h = content ? content->getHeight() : 0; if(h<=0) h=720;
    return new DrawView(w,h,std::move(d));
}

class CDCONTEXT:public testing::Test{
public :
   virtual void SetUp(){}
   virtual void TearDown(){}
};

TEST_F(CDCONTEXT,TEXT_ALIGNMENT){
    GUIEnvironment::content()->addView(newDrawView([](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        cdroid::Rect rect={100,100,800,120};
        ctx.set_font_size(40);
        ctx.set_color(0xFFFFFFFF); ctx.rectangle(0,0,1280,720); ctx.fill();
        ctx.set_color(0xFFFF0000); ctx.rectangle(rect); ctx.fill();
        ctx.set_color(0xFF00FF00);
        ctx.draw_text(rect,"The quick brown fox jump sover the lazy dog.",(2<<4)|2);
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,circle){
    const int PTCOUNT=30;
    const double RADIUS=280;
    struct CPoint{ double x; double y; };
    CPoint pts[PTCOUNT];
    for(int i=0;i<PTCOUNT;i++){
        pts[i].x=RADIUS*cos(M_PI*2.f*i/PTCOUNT);
        pts[i].y=RADIUS*sin(M_PI*2.f*i/PTCOUNT);
    }
    GUIEnvironment::content()->addView(newDrawView([=](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        ctx.set_source_rgb(.1,.1,.1);
        ctx.rectangle(0,0,800,600);
        ctx.fill();
        ctx.translate(300,300);
        ctx.set_source_rgb(0,1,0);
        ctx.arc(0,0,RADIUS,0,M_PI*2.);
        ctx.stroke();
        for(int i=0;i<PTCOUNT;i++){
            ctx.set_source_rgba((float)i/PTCOUNT,(float)(PTCOUNT-i)/PTCOUNT,0,0.4);
            for(int j=i+1;j<PTCOUNT;j++){
                ctx.move_to(pts[i].x,pts[i].y);
                ctx.line_to(pts[j].x,pts[j].y);
            }
            ctx.stroke();
        }
        ctx.translate(-300,-300);
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,Translate){
    GUIEnvironment::content()->addView(newDrawView([](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        ctx.set_color(0xFFFFFFFF); ctx.rectangle(0,0,800,600); ctx.fill();
        ctx.set_color(0xFFFF0000); ctx.rectangle(0,0,200,200); ctx.fill();
        ctx.translate(100,100);
        ctx.set_color(0xFF00FF00); ctx.rectangle(0,0,100,100); ctx.fill();
        ctx.translate(-50,-50);
        ctx.set_color(0xFF0000FF); ctx.rectangle(0,0,100,100); ctx.fill();
        ctx.translate(-50,-50);
        ctx.set_color(0xFFFFFF00); ctx.rectangle(0,0,100,100); ctx.stroke();
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,Clip){
    cdroid::RefPtr<ImageSurface>img=loadFixturePng("light.png");
    GUIEnvironment::content()->addView(newDrawView([img](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        cdroid::Rect rect={0,0,800,600};
        ctx.set_color(0xFFFFFFFF); ctx.rectangle(0,0,800,600); ctx.fill();
        ctx.arc(400,300,100,0,M_PI*2);
        ctx.clip();
        ctx.draw_image(img,rect,nullptr);
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,Clip1){
    GUIEnvironment::content()->addView(newDrawView([](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        ctx.set_color(0xFF888888); ctx.rectangle(-100,-100,1280,720); ctx.fill();
        ctx.rectangle(10,10,60,60);
        ctx.rectangle(100,100,400,400);
        ctx.clip();
        ctx.save();
        ctx.rectangle(200,200,400,200);
        ctx.clip();
        ctx.set_source_rgba(1,0,0,.5f);
        ctx.rectangle(0,0,1280,720);
        ctx.fill();
        ctx.restore();
        ctx.rectangle(-100,-100,1280,720);
        ctx.set_source_rgba(0,0,1,0.5f);
        ctx.fill();
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,Clip2){
    GUIEnvironment::content()->addView(newDrawView([](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        ctx.reset_clip();
        ctx.arc(100,100,50,0,M_PI*2.f);
        ctx.clip();
        double x1=0,y1=0,x2=0,y2=0;
        ctx.get_clip_extents(x1,y1,x2,y2);
        std::vector<Cairo::Rectangle>lst;
        /* A curved clip has no rectangle-list representation by cairo's
           design (CLIP_NOT_REPRESENTABLE); reset the clip so the state does
           not leak into later draws and take the empty list. */
        try { ctx.copy_clip_rectangle_list(lst); }
        catch (const std::exception&) { ctx.reset_clip(); lst.clear(); }
        printf("CLIPS(%f,%f,%f,%f) lst.size=%lu\r\n",x1,y1,x2,y2,(unsigned long)lst.size());
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,Clip3){
    GUIEnvironment::content()->addView(newDrawView([](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        ctx.reset_clip();
        ctx.rectangle(0,0,200,200);
        ctx.clip();
        double cx1,cy1,cx2,cy2;
        ctx.get_clip_extents(cx1,cy1,cx2,cy2);
        printf("clip region before translate(%.f,%.f,%.f,%.f)\r\n",cx1,cy1,cx2,cy2);
        ctx.translate(100,100);
        ctx.get_clip_extents(cx1,cy1,cx2,cy2);
        printf("clip region after  translate(%.f,%.f,%.f,%.f)\r\n",cx1,cy1,cx2,cy2);
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,Mask){
    cdroid::RefPtr<ImageSurface>img=loadFixturePng("im_game.png");
    GUIEnvironment::content()->addView(newDrawView([img](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        cdroid::RefPtr<Pattern>pat1=SurfacePattern::create(img);
        cdroid::RefPtr<Pattern>pat2=SolidPattern::create_rgba(0,1.0,0,.5);
        ctx.set_source(pat1);
        ctx.rectangle(0,0,800,600);
        ctx.mask(pat2);
        ctx.paint();
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,ImageSurface){
    cdroid::RefPtr<ImageSurface>img=loadFixturePng("im_game.png");
    GUIEnvironment::content()->addView(newDrawView([img](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        cdroid::RefPtr<Cairo::Context>ictx=Cairo::Context::create(img);
        cdroid::RefPtr<Gradient>pat=LinearGradient::create(0,0,400,0);
        cdroid::RefPtr<Pattern>pat1=SolidPattern::create_rgba(1,0,0,.51);
        pat->add_color_stop_rgba ( .0, 1, 1, 1, 0);
        pat->add_color_stop_rgba ( .2, 0, 1, 0, 0.5);
        pat->add_color_stop_rgba ( .4, 1, 1, 1, 0);
        pat->add_color_stop_rgba ( .6, 0, 0, 1, 0.5);
        pat->add_color_stop_rgba ( .8, 1, 1, 1, 0);
        ictx->set_source(pat1);
        ictx->rectangle(0,0,400,400);
        ictx->fill();
        ctx.set_source(img,0,0);
        ctx.rectangle(0,0,400,400);
        ctx.fill();
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,Pattern_Line){
    GUIEnvironment::content()->addView(newDrawView([](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        int i, j;
        cdroid::RefPtr<RadialGradient>radpat(RadialGradient::create(200, 150, 80, 400, 300, 400));
        cdroid::RefPtr<LinearGradient>linpat(LinearGradient::create(200, 210, 600, 390));
        radpat->add_color_stop_rgb ( 0, 1.0, 0.8, 0.8);
        radpat->add_color_stop_rgb ( 1, 0.9, 0.0, 0.0);
        for (i=1; i<10; i++)
             for (j=1; j<10; j++)
                 ctx.rectangle ( i*50, j*50,48 ,48);
        ctx.set_source ( radpat);
        ctx.fill ();
        linpat->add_color_stop_rgba ( .0, 1, 1, 1, 0);
        linpat->add_color_stop_rgba ( .2, 0, 1, 0, 0.5);
        linpat->add_color_stop_rgba ( .4, 1, 1, 1, 0);
        linpat->add_color_stop_rgba ( .6, 0, 0, 1, 0.5);
        linpat->add_color_stop_rgba ( .8, 1, 1, 1, 0);
        ctx.rectangle ( 0, 0, 800, 600);
        ctx.set_source(linpat);
        ctx.fill ();
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,Pattern_Radio){
    GUIEnvironment::content()->addView(newDrawView([](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        cdroid::RefPtr<RadialGradient>radpat(RadialGradient::create(200, 200, 10, 200, 200, 150));
        radpat->add_color_stop_rgb ( .0, 1., 1., 1.);
        radpat->add_color_stop_rgb ( 1., 1., .0,.0);
        ctx.set_source ( radpat);
        ctx.arc(200,200,150,.0,3.1415*2);
        ctx.fill ();
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,Font){
    const char *texts[]={
        "The quick brown fox jumps over a lazy dog",
        "Innovation in China 0123456789"
    };
    GUIEnvironment::content()->addView(newDrawView([=](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        ctx.set_color(0xffffffff); ctx.rectangle(0,0,1280,720); ctx.fill();
        int y=10;
        for(int i=0;i<10;i++){
           std::string txt=texts[i%(int)(sizeof(texts)/sizeof(char*))];
           int font_size=10+i*5;
           ctx.set_font_size(font_size);
           ctx.move_to(0,y);y+=font_size+10;
           if(i%4==0)ctx.set_color(0,0,0);
           else{
               auto lg=LinearGradient::create(10,480,1100,720);
               lg->add_color_stop_rgba(.0,1.0,0,0,0.5);
               lg->add_color_stop_rgba(.5,.0,1.,.0,1.);
               lg->add_color_stop_rgba(1.,.0,0,.1,0.5);
               ctx.set_source(lg);
           }
           if(i%2==0) ctx.show_text(txt);
           else{ ctx.text_path(txt); ctx.stroke(); }
        }
        ctx.set_font_size(150);
        auto lg=LinearGradient::create(20,480,300,720);
        lg->add_color_stop_rgba(.0,1.0,0,0,0.5);
        lg->add_color_stop_rgba(.5,.0,1.,.0,1.);
        lg->add_color_stop_rgba(1.,.0,0,.1,0.5);
        ctx.set_source(lg);
        ctx.move_to(0,650);
        ctx.show_text("Innovation in China!");
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,ALPHA){
    GUIEnvironment::content()->addView(newDrawView([](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        ctx.set_color(0xFFFFFFFF); ctx.rectangle(0,0,1280,720); ctx.fill();
        ctx.set_color(0x80FF0000); ctx.rectangle(200,200,480,320); ctx.fill();
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,Hole){
    GUIEnvironment::content()->addView(newDrawView([](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        ctx.set_color(0xFFFFFFFF); ctx.rectangle(0,0,1280,720); ctx.fill();
        ctx.set_color(0); ctx.rectangle(200,200,480,320); ctx.fill();
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,Hole2){
    GUIEnvironment::content()->addView(newDrawView([](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        ctx.set_color(0xFFFFFFFF); ctx.rectangle(0,0,1280,720); ctx.fill();
        ctx.set_source_rgba(0,0,0,0); ctx.rectangle(200,200,480,320); ctx.fill();
    }));
    pumpFor(1500);
}

TEST_F(CDCONTEXT,Hole3){
    GUIEnvironment::content()->addView(newDrawView([](Canvas&ctx){
            CanvasStateGuard _canvasGuard(ctx);
        cdroid::RefPtr<ImageSurface>img=ImageSurface::create(Surface::Format::ARGB32,1280,720);
        cdroid::RefPtr<Cairo::Context>ctx1=Cairo::Context::create(img);
        ctx1->set_source_rgb(1,0.5,1);
        ctx1->rectangle(0,0,1280,720);
        ctx1->fill();
        ctx1->set_source_rgba(0,1,0,0.1);
        ctx1->rectangle(200,200,480,320);
        ctx1->fill();
        ctx.set_source(img,0,0);
        ctx.rectangle(0,0,1280,720);
        ctx.fill();
    }));
    pumpFor(1500);
}

// ===========================================================================
// AOSP app-data directories + manifest package (docs/app-data-directory-layout-plan.md)
// ===========================================================================
#include <cstdio>
#include <sys/stat.h>
#include <cstdlib>
#include <unistd.h>
#include <content/sharedpreferences.h>

TEST_F(CDCONTEXT, PackageNameIsStableManifestId) {
    const std::string pkg = App::getInstance().getPackageName();
    EXPECT_FALSE(pkg.empty());
    // A package id is never a path (the old text-era behavior returned the
    // exe path, which leaked into prefs/dirs as nested directory trees).
    EXPECT_EQ(std::string::npos, pkg.find('/'));
    EXPECT_EQ(std::string::npos, pkg.find('\\'));
}

TEST_F(CDCONTEXT, DataDirsFollowAospShape) {
    App& app = App::getInstance();
    const std::string pkg = app.getPackageName();
    const std::string dataDir = app.getDataDir();
    EXPECT_NE(std::string::npos, dataDir.find("/data/" + pkg));
    EXPECT_NE(std::string::npos, app.getFilesDir().find(dataDir + "/files"));
    EXPECT_NE(std::string::npos, app.getCacheDir().find(dataDir + "/cache"));
    // AOSP ContextImpl.getDir prefixes "app_".
    EXPECT_NE(std::string::npos, app.getDir("probe", 0).find(dataDir + "/app_probe"));
    const std::string prefsPath = app.getSharedPreferencesPath("probe_prefs");
    EXPECT_NE(std::string::npos, prefsPath.find(dataDir + "/shared_prefs/probe_prefs.xml"));
    EXPECT_NE(std::string::npos, app.getFileStreamPath("f.txt").find("/files/f.txt"));
    // External resolves through the Environment builders (emulated volume).
    EXPECT_NE(std::string::npos, app.getExternalFilesDir().find("Android/data/" + pkg + "/files"));
}

TEST_F(CDCONTEXT, PrefsReadLegacyFlatStore) {
    // Pre-migration flat store (~/.cdroid/prefs/<n>.xml) is a read fallback;
    // the first write migrates the content to shared_prefs/.
    const char* home = getenv("HOME");
    ASSERT_NE(nullptr, home);
    const std::string legacyDir = std::string(home) + "/.cdroid/prefs";
    mkdir(legacyDir.c_str(), 0755);
    const std::string legacyFile = legacyDir + "/legacyprobe.xml";
    FILE* f = fopen(legacyFile.c_str(), "w");
    ASSERT_NE(nullptr, f);
    fputs("<?xml version='1.0' encoding='utf-8' standalone='yes' ?>\n"
          "<map>\n    <string name=\"legacy_key\">legacy_value</string>\n</map>\n", f);
    fclose(f);

    auto sp = App::getInstance().getSharedPreferences("legacyprobe", 0);
    EXPECT_EQ("legacy_value", sp->getString("legacy_key", ""));
    // First write lands under the per-app shared_prefs/ tree.
    sp->edit().putString("migrated_key", "new_home").commit();
    const std::string newPath = App::getInstance().getSharedPreferencesPath("legacyprobe");
    FILE* nf = fopen(newPath.c_str(), "r");
    ASSERT_NE(nullptr, nf) << newPath;
    fclose(nf);
    unlink(legacyFile.c_str());
    unlink(newPath.c_str());
}
