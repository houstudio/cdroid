/* tintdemo — verify Drawable tint + BlendMode on cairo.
 *
 * Section 1: PorterDuff::Mode via setTint/setTintMode (the mTintFilter path,
 *   driven by Drawable::begin/endTintGroup).
 * Section 2: BlendMode via setColorFilter(BlendModeColorFilter) (the mColorFilter
 *   path) — exercises the EXTENDED modes PorterDuff can't express (COLOR_DODGE,
 *   COLOR_BURN, HARD/SOFT_LIGHT, DIFFERENCE, EXCLUSION, 4 HSL, PLUS, MODULATE).
 *   cairo operators are W3C == Android BlendMode, so these should be exact.
 *
 * Build: from outX64-Debug, `make` then `make tintdemo`.
 */
#include <cdroid.h>
#include <cdlog.h>
#include <core/porterduff.h>
#include <drawable/colorfilters.h>
#include <functional>

struct Mode{ const char* name; int mode; };

static void addCell(Window*w,App&app,const std::string&res,int x,int y,int sz,
                    const char*name,std::function<void(Drawable*)>apply){
    ImageView*iv=new ImageView(sz,sz);
    Drawable*dr=app.getDrawable(res);
    if(dr) dr=dr->mutate();
    iv->setImageDrawable(dr);
    if(dr) apply(dr);
    iv->setBackgroundColor(0xFF1b2330);
    iv->setScaleType(ScaleType::FIT_CENTER);
    w->addView(iv);
    iv->layout(x,y,sz,sz);
    TextView*lbl=new TextView(0,0);
    lbl->setText(name);
    lbl->setTextSize(12);
    lbl->setGravity(Gravity::CENTER);
    lbl->setTextColor(0xFFcfd8dc);
    w->addView(lbl);
    lbl->layout(x,y+sz+1,sz,18);
}

static int addSection(Window*w,const char*title,int y){
    TextView*h=new TextView(0,0);
    h->setText(title);
    h->setTextSize(14);
    h->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
    h->setTextColor(0xFF8aa0b4);
    w->addView(h);
    h->layout(20,y,984,22);
    return y+26;
}

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    w->setId(1);
    w->setBackgroundColor(0xFF10141c);

    const int tintColor=0xFFFF4040; /* red */
    const std::string res=(argc>1)?std::string(argv[1]):std::string("cdroid:mipmap/ic_search");
    const int sz=78, gap=6, labelH=18, cellW=sz+gap, cellH=sz+labelH+gap, cols=8;
    const int ox=24;

    /* ---- Section 1: PorterDuff::Mode tint (setTint path) ---- */
    int y=addSection(w,"PorterDuff::Mode  (setTint / setTintMode  ->  mTintFilter)",6);
    const Mode pd[]={
        {"(none)",PorterDuff::NOOP},{"SRC",PorterDuff::SRC},{"SRC_IN",PorterDuff::SRC_IN},
        {"SRC_OVER",PorterDuff::SRC_OVER},{"SRC_ATOP",PorterDuff::SRC_ATOP},{"SRC_OUT",PorterDuff::SRC_OUT},
        {"DST_OUT",PorterDuff::DST_OUT},{"MULTIPLY",PorterDuff::MULTIPLY},{"SCREEN",PorterDuff::SCREEN},
        {"OVERLAY",PorterDuff::OVERLAY},{"DARKEN",PorterDuff::DARKEN},{"LIGHTEN",PorterDuff::LIGHTEN},
        {"ADD",PorterDuff::ADD},{"CLEAR",PorterDuff::CLEAR},
    };
    for(size_t i=0;i<sizeof(pd)/sizeof(pd[0]);i++){
        const int col=i%cols,row=i/cols;
        const int m=pd[i].mode;
        addCell(w,app,res, ox+col*cellW, y+row*cellH, sz, pd[i].name,
            [tintColor,m](Drawable*d){ if(m!=PorterDuff::NOOP){ d->setTint(tintColor); d->setTintMode(m);} });
    }
    y += ((sizeof(pd)/sizeof(pd[0])+cols-1)/cols)*cellH + 6;

    /* ---- Section 2: BlendMode (setColorFilter + BlendModeColorFilter) ---- */
    y=addSection(w,"BlendMode  (setColorFilter(BlendModeColorFilter)  ->  mColorFilter)  [extended modes]",y);
    const Mode bm[]={
        {"SRC_IN",BlendMode::SRC_IN},{"MULTIPLY",BlendMode::MULTIPLY},      /* sanity vs section 1 */
        {"PLUS",BlendMode::PLUS},{"MODULATE",BlendMode::MODULATE},
        {"COLOR_DODGE",BlendMode::COLOR_DODGE},{"COLOR_BURN",BlendMode::COLOR_BURN},
        {"HARD_LIGHT",BlendMode::HARD_LIGHT},{"SOFT_LIGHT",BlendMode::SOFT_LIGHT},
        {"DIFFERENCE",BlendMode::DIFFERENCE},{"EXCLUSION",BlendMode::EXCLUSION},
        {"HUE",BlendMode::HUE},{"SATURATION",BlendMode::SATURATION},
        {"COLOR",BlendMode::COLOR},{"LUMINOSITY",BlendMode::LUMINOSITY},
    };
    for(size_t i=0;i<sizeof(bm)/sizeof(bm[0]);i++){
        const int col=i%cols,row=i/cols;
        const int m=bm[i].mode;
        addCell(w,app,res, ox+col*cellW, y+row*cellH, sz, bm[i].name,
            [tintColor,m](Drawable*d){ d->setColorFilter(std::make_shared<BlendModeColorFilter>(tintColor,m)); });
    }

    w->requestLayout();
    return app.exec();
}
