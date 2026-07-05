/* tintdemo — verify Drawable tint works for every PorterDuff/blend mode on cairo.
 *
 * A grid of cells: the same drawable tinted with one PorterDuff::Mode per cell
 * (red tint), via the mTintFilter path (Drawable::setTint + setTintMode) that
 * Drawable::begin/endTintGroup drives. Compare against Android.
 *
 * Default drawable is a transparent magnifier icon (cdroid:mipmap/ic_search) so
 * the alpha-dependent modes (SRC_IN shows the red icon shape, SRC_OUT its inverse,
 * CLEAR/DST_OUT go blank, MULTIPLY/SCREEN/... recolor) are all distinguishable.
 * Pass any drawable resource as argv[1] to swap it.
 *
 * Build: from outX64-Debug, `make` then `make tintdemo`.
 */
#include <cdroid.h>
#include <cdlog.h>
#include <core/porterduff.h>

struct TintCase{ const char* name; int mode; };

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    w->setId(1);
    w->setBackgroundColor(0xFF10141c);

    const int tintColor=0xFFFF4040; /* red — MULTIPLY darkens, SRC_IN recolors, etc. */
    const TintCase cases[]={
        {"(none)",   PorterDuff::NOOP},
        {"SRC",      PorterDuff::SRC},
        {"SRC_IN",   PorterDuff::SRC_IN},
        {"SRC_OVER", PorterDuff::SRC_OVER},
        {"SRC_ATOP", PorterDuff::SRC_ATOP},
        {"SRC_OUT",  PorterDuff::SRC_OUT},
        {"DST_OUT",  PorterDuff::DST_OUT},
        {"MULTIPLY", PorterDuff::MULTIPLY},
        {"SCREEN",   PorterDuff::SCREEN},
        {"OVERLAY",  PorterDuff::OVERLAY},
        {"DARKEN",   PorterDuff::DARKEN},
        {"LIGHTEN",  PorterDuff::LIGHTEN},
        {"ADD",      PorterDuff::ADD},
        {"CLEAR",    PorterDuff::CLEAR},
    };
    const int N=int(sizeof(cases)/sizeof(cases[0]));
    const int iconSize=96, labelH=26, gap=8;
    const int cellW=iconSize+gap, cellH=iconSize+labelH+gap, cols=7;
    const int ox=16, oy=16;
    const std::string res=(argc>1)?std::string(argv[1]):std::string("cdroid:mipmap/ic_search");

    for(int i=0;i<N;i++){
        const int col=i%cols, row=i/cols;
        const int x=ox+col*cellW, y=oy+row*cellH;
        ImageView*iv=new ImageView(iconSize,iconSize);
        Drawable*dr=app.getDrawable(res);
        if(dr) dr=dr->mutate();
        iv->setImageDrawable(dr);
        if(dr && cases[i].mode!=PorterDuff::NOOP){
            dr->setTint(tintColor);          /* -> mTintFilter (PorterDuffColorFilter) */
            dr->setTintMode(cases[i].mode);
        }
        iv->setBackgroundColor(0xFF1b2330);   /* dark bg so alpha effects are visible */
        iv->setScaleType(ScaleType::FIT_CENTER);
        w->addView(iv);
        iv->layout(x,y,iconSize,iconSize);

        TextView*lbl=new TextView(0,0);
        lbl->setText(cases[i].name);
        lbl->setTextSize(13);
        lbl->setGravity(Gravity::CENTER);
        lbl->setTextColor(0xFFcfd8dc);
        w->addView(lbl);
        lbl->layout(x,y+iconSize+2,iconSize,labelH);

        LOGI("cell[%d] %-9s mode=%d",i,cases[i].name,cases[i].mode);
    }
    w->requestLayout();
    return app.exec();
}
