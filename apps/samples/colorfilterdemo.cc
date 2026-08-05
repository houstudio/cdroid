/* colorfilterdemo — verify ColorFilter implementations (per-pixel, on the drawable's ARGB32 group).
 *
 * LightingColorFilter   : per-channel mul+add (alpha preserved) — Skia premultiplied form.
 * ColorMatrixColorFilter: full 4x5 matrix (0..255, unpremul -> matrix -> re-premul).
 *
 * Both are applied via ImageView.setColorFilter(), which drives Drawable::begin/endTintGroup:
 * the drawable draws into a pushed ARGB32 group, then the filter's apply() mutates that group's
 * pixels in place, then the group is composited back. Source is a rainbow gradient so per-channel
 * and matrix effects are clearly visible.
 *
 * Build: from outX64-Debug, `make colorfilterdemo`.
 */
#include <cdroid.h>
#include <cdlog.h>
#include <drawable/colorfilters.h>
#include <drawable/colormatrix.h>
#include <drawable/gradientdrawable.h>
#include <functional>

struct Filt{ const char* name; std::function<void(Drawable*)> apply; };

static Drawable* rainbow(){
    return new GradientDrawable(GradientDrawable::LEFT_RIGHT,
        std::vector<int>{(int)0xFFFF0000,(int)0xFFFFFF00,(int)0xFF00FF00,
                         (int)0xFF00FFFF,(int)0xFF0000FF,(int)0xFFFF00FF});
}

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    w->setBackgroundColor(0xFF10141c);

    ScrollView*scroller=new ScrollView(-1,-1);
    scroller->setVerticalScrollBarEnabled(true);
    w->addView(scroller);
    LinearLayout*content=new LinearLayout(-1,-2);
    content->setOrientation(LinearLayout::VERTICAL);
    scroller->addView(content);

    const int sz=100, gap=6, labelH=20, cols=6;
    auto lp=[&](int ww,int hh,int lm=0,int tm=0){
        LinearLayout::LayoutParams*p=new LinearLayout::LayoutParams(ww,hh);
        p->leftMargin=lm; p->topMargin=tm; return p;
    };
    auto addSection=[&](const char*title){
        TextView*h=new TextView(0,0); h->setText(title); h->setTextSize(14);
        h->setTextColor(0xFF8aa0b4); content->addView(h, lp(-1,24,24,8));
    };
    auto newRow=[&](){
        LinearLayout*r=new LinearLayout(-1,-2); r->setOrientation(LinearLayout::HORIZONTAL);
        content->addView(r, lp(-1,-2,0,2)); return r;
    };
    auto addCell=[&](LinearLayout*row,const char*name,std::function<void(Drawable*)>apply){
        LinearLayout*cell=new LinearLayout(sz,sz+labelH); cell->setOrientation(LinearLayout::VERTICAL);
        ImageView*iv=new ImageView(sz,sz);
        Drawable*dr=rainbow()->mutate();
        iv->setImageDrawable(dr);
        if(dr) apply(dr);
        iv->setBackgroundColor(0xFF1b2330);
        iv->setScaleType(ScaleType::FIT_XY);
        cell->addView(iv, lp(sz,sz));
        TextView*lbl=new TextView(0,0); lbl->setText(name); lbl->setTextSize(11);
        lbl->setGravity(Gravity::CENTER); lbl->setTextColor(0xFFcfd8dc);
        cell->addView(lbl, lp(sz,labelH));
        row->addView(cell, lp(sz,-2,gap,0));
    };

    /* ---- LightingColorFilter ---- */
    addSection("LightingColorFilter   R'=R*mul.R+add.R per channel (alpha kept)");
    const Filt lc[]={
        {"(none)",    [](Drawable*){ /* unfiltered reference */ }},
        {"mul RED",   [](Drawable*d){ d->setColorFilter(std::make_shared<LightingColorFilter>(0xFF0000,0)); }},
        {"mul GREEN", [](Drawable*d){ d->setColorFilter(std::make_shared<LightingColorFilter>(0x00FF00,0)); }},
        {"mul BLUE",  [](Drawable*d){ d->setColorFilter(std::make_shared<LightingColorFilter>(0x0000FF,0)); }},
        {"darken .5", [](Drawable*d){ d->setColorFilter(std::make_shared<LightingColorFilter>(0x808080,0)); }},
        {"add +0x40", [](Drawable*d){ d->setColorFilter(std::make_shared<LightingColorFilter>(0xFFFFFF,0x404040)); }},
        {"mulY addB", [](Drawable*d){ d->setColorFilter(std::make_shared<LightingColorFilter>(0xFFFF00,0x0000FF)); }},
        {"mul+add",   [](Drawable*d){ d->setColorFilter(std::make_shared<LightingColorFilter>(0xCCCCCC,0x200010)); }},
    };
    LinearLayout*row=nullptr;
    for(size_t i=0;i<sizeof(lc)/sizeof(lc[0]);i++){ if(i%cols==0) row=newRow(); addCell(row,lc[i].name,lc[i].apply); }

    /* ---- ColorMatrixColorFilter ---- */
    addSection("ColorMatrixColorFilter   4x5 matrix");
    const Filt cm[]={
        {"grayscale", [](Drawable*d){ ColorMatrix m; m.setSaturation(0.f);   d->setColorFilter(std::make_shared<ColorMatrixColorFilter>(m.mArray)); }},
        {"sat .5",    [](Drawable*d){ ColorMatrix m; m.setSaturation(0.5f);  d->setColorFilter(std::make_shared<ColorMatrixColorFilter>(m.mArray)); }},
        {"sat 2.0",   [](Drawable*d){ ColorMatrix m; m.setSaturation(2.0f);  d->setColorFilter(std::make_shared<ColorMatrixColorFilter>(m.mArray)); }},
        {"invert",    [](Drawable*d){ const float m[20]={-1,0,0,0,255, 0,-1,0,0,255, 0,0,-1,0,255, 0,0,0,1,0}; d->setColorFilter(std::make_shared<ColorMatrixColorFilter>(m)); }},
        {"green only",[](Drawable*d){ const float m[20]={0,0,0,0,0, 0,1,0,0,0, 0,0,0,0,0, 0,0,0,1,0}; d->setColorFilter(std::make_shared<ColorMatrixColorFilter>(m)); }},
        {"swap R/B",  [](Drawable*d){ const float m[20]={0,0,1,0,0, 0,1,0,0,0, 1,0,0,0,0, 0,0,0,1,0}; d->setColorFilter(std::make_shared<ColorMatrixColorFilter>(m)); }},
        {"red bias",  [](Drawable*d){ const float m[20]={1,0,0,0,90, 0,1,0,0,0, 0,0,1,0,0, 0,0,0,1,0}; d->setColorFilter(std::make_shared<ColorMatrixColorFilter>(m)); }},
        {"alpha .5",  [](Drawable*d){ const float m[20]={1,0,0,0,0, 0,1,0,0,0, 0,0,1,0,0, 0,0,0,0.5f,0}; d->setColorFilter(std::make_shared<ColorMatrixColorFilter>(m)); }},
    };
    row=nullptr;
    for(size_t i=0;i<sizeof(cm)/sizeof(cm[0]);i++){ if(i%cols==0) row=newRow(); addCell(row,cm[i].name,cm[i].apply); }

    content->requestLayout();
    return app.exec();
}
