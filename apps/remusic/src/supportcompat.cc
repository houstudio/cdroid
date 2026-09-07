// Support-library tag aliases: the original layouts address widgets through
// their android.support / design / MagicaSakura / Fresco / app class paths.
// The LayoutInflater registry keys on the SHORT class name (the segment after
// the last '.'), so RecyclerView/DrawerLayout/ViewPager/TabLayout/
// NavigationView already resolve to the framework ports' own registrations.
// The views with no CDROID counterpart are re-registered here under their
// short names; where no port exists the view degrades to the closest plain
// container.
#include <cdroid.h>
#include <widget/imageview.h>
#include <widget/framelayout.h>
#include <widget/progressbar.h>
#include <widget/seekbar.h>
#include <widget/toolbar.h>
#include <widget/viewpager.h>

using namespace cdroid;

namespace {

template <typename T>
void alias(const char* xmlName) {
    // Ignore the duplicate-registration refusal: framework classes that share
    // the short name keep their own, richer inflater.
    LayoutInflater::registerInflater(xmlName, 0, [](Context* ctx, const AttributeSet& attr) -> View* {
        return new T(ctx, &attr);
    });
}

const bool sRegistered = [] {
    // MagicaSakura Tint* wrappers: theming handled by the palette swap instead.
    alias<Toolbar>("TintToolbar");
    alias<ImageView>("TintImageView");
    alias<ProgressBar>("TintProgressBar");
    // Fresco SimpleDraweeView: album-art path; placeholder attrs parsed by a
    // DraweeView stand-in later — plain ImageView for now.
    alias<ImageView>("SimpleDraweeView");
    // No SwipeRefreshLayout port: degrade to a plain FrameLayout container.
    alias<FrameLayout>("SwipeRefreshLayout");
    // com.wm.remusic.widget.CustomViewPager: non-scrolling ViewPager variant;
    // plain ViewPager until a no-swipe subclass is needed.
    alias<ViewPager>("CustomViewPager");
    // com.wm.remusic.widget.PlayerSeekBar: themed seekbar — plain SeekBar.
    LayoutInflater::registerInflater("PlayerSeekBar", 0, [](Context* ctx, const AttributeSet& attr) -> View* {
        return new SeekBar(ctx, &attr);
    });
    // com.wm.remusic.widget.SideBar: A-Z index bar — placeholder View stub.
    LayoutInflater::registerInflater("SideBar", 0, [](Context* ctx, const AttributeSet& attr) -> View* {
        return new View(ctx, &attr);
    });

    // FQCN twins: getInflater resolves a dotted tag EXACTLY first (AOSP
    // createViewFromTag never strips a dotted name), so the original library
    // spellings in the layouts hit these without the folding fingerprint.
    // Spellings mirror the layouts verbatim.
    alias<Toolbar>("com.bilibili.magicasakura.widgets.TintToolbar");
    alias<ImageView>("com.bilibili.magicasakura.widgets.TintImageView");
    alias<ProgressBar>("com.bilibili.magicasakura.widgets.TintProgressBar");
    alias<ImageView>("com.facebook.drawee.view.SimpleDraweeView");
    alias<FrameLayout>("android.support.v4.widget.SwipeRefreshLayout");
    alias<ViewPager>("com.wm.remusic.widget.CustomViewPager");
    LayoutInflater::registerInflater("com.wm.remusic.widget.PlayerSeekBar", 0, [](Context* ctx, const AttributeSet& attr) -> View* {
        return new SeekBar(ctx, &attr);
    });
    LayoutInflater::registerInflater("com.wm.remusic.widget.SideBar", 0, [](Context* ctx, const AttributeSet& attr) -> View* {
        return new View(ctx, &attr);
    });
    return true;
}();

} // namespace
