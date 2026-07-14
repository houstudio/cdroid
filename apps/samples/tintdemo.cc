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
 *
 * NOTE: the cells form a grid. Window::doLayout now always lays out direct children,
 * so the old absolute layout() grid piled up at (0,0); the grid is now built from
 * nested LinearLayouts (rows of cells) inside a ScrollView.
 */
#include <cdroid.h>
#include <cdlog.h>
#include <core/porterduff.h>
#include <drawable/colorfilters.h>
#include <functional>

struct Mode{ const char* name; int mode; };

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    w->setId(1);
    w->setBackgroundColor(0xFF10141c);

    ScrollView*scroller=new ScrollView(-1,-1);
    scroller->setVerticalScrollBarEnabled(true);
    w->addView(scroller);
    LinearLayout*content=new LinearLayout(-1,-2); // MATCH_PARENT w, WRAP_CONTENT h
    content->setOrientation(LinearLayout::VERTICAL);
    scroller->addView(content);

    const int tintColor=0xFFFF4040; /* red */
    const std::string res=(argc>1)?std::string(argv[1]):std::string("cdroid:mipmap/ic_search");
    const int sz=78, gap=6, labelH=18, cols=8;

    auto lp=[&](int ww,int hh,int lmargin=0,int tmargin=0){
        LinearLayout::LayoutParams*p=new LinearLayout::LayoutParams(ww,hh);
        p->leftMargin=lmargin; p->topMargin=tmargin; return p;
    };
    auto addSection=[&](const char*title){
        TextView*h=new TextView(0,0);
        h->setText(title);
        h->setTextSize(14);
        h->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
        h->setTextColor(0xFF8aa0b4);
        content->addView(h, lp(-1,22,24,8));
    };
    auto newRow=[&](){
        LinearLayout*r=new LinearLayout(-1,-2);
        r->setOrientation(LinearLayout::HORIZONTAL);
        content->addView(r, lp(-1,-2,0,2));
        return r;
    };
    auto addCell=[&](LinearLayout*row,const char*name,std::function<void(Drawable*)>apply){
        LinearLayout*cell=new LinearLayout(sz,sz+labelH);
        cell->setOrientation(LinearLayout::VERTICAL);
        ImageView*iv=new ImageView(sz,sz);
        Drawable*dr=app.getDrawable(res);
        if(dr) dr=dr->mutate();
        iv->setImageDrawable(dr);
        if(dr) apply(dr);
        iv->setBackgroundColor(0xFF1b2330);
        iv->setScaleType(ScaleType::FIT_CENTER);
        cell->addView(iv, lp(sz,sz));
        TextView*lbl=new TextView(0,0);
        lbl->setText(name);
        lbl->setTextSize(12);
        lbl->setGravity(Gravity::CENTER);
        lbl->setTextColor(0xFFcfd8dc);
        cell->addView(lbl, lp(sz,labelH));
        row->addView(cell, lp(sz,-2,gap,0));
    };

    /* ---- Section 1: PorterDuff::Mode tint (setTint path) ---- */
    addSection("PorterDuff::Mode  (setTint / setTintMode  ->  mTintFilter)");
    const Mode pd[]={
        {"(none)",PorterDuff::NOOP},{"SRC",PorterDuff::SRC},{"SRC_IN",PorterDuff::SRC_IN},
        {"SRC_OVER",PorterDuff::SRC_OVER},{"SRC_ATOP",PorterDuff::SRC_ATOP},{"SRC_OUT",PorterDuff::SRC_OUT},
        {"DST_OUT",PorterDuff::DST_OUT},{"MULTIPLY",PorterDuff::MULTIPLY},{"SCREEN",PorterDuff::SCREEN},
        {"OVERLAY",PorterDuff::OVERLAY},{"DARKEN",PorterDuff::DARKEN},{"LIGHTEN",PorterDuff::LIGHTEN},
        {"ADD",PorterDuff::ADD},{"CLEAR",PorterDuff::CLEAR},
    };
    LinearLayout*row=nullptr;
    for(size_t i=0;i<sizeof(pd)/sizeof(pd[0]);i++){
        if(i%cols==0) row=newRow();
        const int m=pd[i].mode;
        addCell(row, pd[i].name,
            [tintColor,m](Drawable*d){ if(m!=PorterDuff::NOOP){ d->setTint(tintColor); d->setTintMode(m);} });
    }

    /* ---- Section 2: BlendMode (setColorFilter + BlendModeColorFilter) ---- */
    addSection("BlendMode  (setColorFilter(BlendModeColorFilter)  ->  mColorFilter)  [extended modes]");
    const Mode bm[]={
        {"SRC_IN",BlendMode::SRC_IN},{"MULTIPLY",BlendMode::MULTIPLY},      /* sanity vs section 1 */
        {"PLUS",BlendMode::PLUS},{"MODULATE",BlendMode::MODULATE},
        {"COLOR_DODGE",BlendMode::COLOR_DODGE},{"COLOR_BURN",BlendMode::COLOR_BURN},
        {"HARD_LIGHT",BlendMode::HARD_LIGHT},{"SOFT_LIGHT",BlendMode::SOFT_LIGHT},
        {"DIFFERENCE",BlendMode::DIFFERENCE},{"EXCLUSION",BlendMode::EXCLUSION},
        {"HUE",BlendMode::HUE},{"SATURATION",BlendMode::SATURATION},
        {"COLOR",BlendMode::COLOR},{"LUMINOSITY",BlendMode::LUMINOSITY},
    };
    row=nullptr;
    for(size_t i=0;i<sizeof(bm)/sizeof(bm[0]);i++){
        if(i%cols==0) row=newRow();
        const int m=bm[i].mode;
        addCell(row, bm[i].name,
            [tintColor,m](Drawable*d){ d->setColorFilter(std::make_shared<BlendModeColorFilter>(tintColor,m)); });
    }

    content->requestLayout();
    return app.exec();
}
