/*********************************************************************************
 * AboutFragment — rich-text intro page (Html.fromHtml): what CDROID is, how the
 * port lines up with AOSP, the feature list, and use cases. The subtitle carries
 * the real framework version (core/build.h).
 *********************************************************************************/
#include <cdroid.h>
#include <core/build.h>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <transition/slide.h>
#include <widget/textview.h>
#include <text/html.h>
#include "printer_common.h"
#include "R.h"

// ---------------------------------------------------------------------------
class AboutFragment : public cdroid::Fragment{
public:
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        cdroid::Fragment::onCreate(savedInstanceState);
        setEnterTransition(new cdroid::Slide(cdroid::Gravity::END));
        setExitTransition(new cdroid::Slide(cdroid::Gravity::END));
    }
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle*) override{
        return inflater->inflate(printerdemo::R::layout::fragment_about, container, false);
    }
    void onViewCreated(cdroid::View* view, cdroid::Bundle*) override{
        cdroid::Fragment::onViewCreated(view, nullptr);
        // Rich text via Html.fromHtml (bold headers, brand-colored bullets/emphasis).
        auto set = [view](int id, const std::string& html){
            cdroid::TextView* tv = (cdroid::TextView*)view->findViewById(id);
            if(tv) tv->setText(cdroid::Html::fromHtml(html));
        };
        // Subtitle carries the real framework version (core/build.h), not a
        // hardcoded app version — the demo ships against whatever CDROID builds.
        if(cdroid::TextView* sub = (cdroid::TextView*)view->findViewById(printerdemo::R::id::about_subtitle))
            sub->setText(std::string("CDroid ") + cdroid::Build::VERSION::RELEASE
                         + " · " + (sLocaleTag.compare(0, 2, "zh") == 0 ? "逐行 C++ 移植 · Android UI"
                                                    : "Line-by-line C++ port · Android UI"));
        set(printerdemo::R::id::about_intro,
            "<big><b><font color='#FF4A90E2'>CDroid<sup>™</sup></font></b></big> 是 <b>Android UI 框架</b>"
            "（android.widget / view / text / drawable / animation）的<b>逐行 C++ 移植</b>，"
            "构建于 <i><b>Cairo</b></i> 矢量图形之上，面向<b>嵌入式设备</b>（最低 <tt>32M</tt> 内存即可运行）。"
            "<br/><br/><blockquote><i>「别再 <s>重造 UI 轮子</s>——把 Android 的真东西移植过来。」</i></blockquote>"
            "<br/><br/>类名、方法签名与控制流<b>紧跟 AOSP</b>——可把 Android 参考源码与 C++ 实现并排逐行对照。"
            "在 Android Studio 设计 XML 布局，<u>直接由 Cairo 渲染</u>，<b>全程无需 JVM</b>。");
        set(printerdemo::R::id::about_arch,
            "<b><font color='#FF4A90E2'>Canvas 即 cairo_t</font></b> —— <small>无独立渲染抽象，也无 Bitmap 类（由 ImageSurface 承担）</small>，<tt>onDraw</tt> 直接编程 cairo。"
            "<br/><b><font color='#FF4A90E2'>App = Context/Assets</font></b> —— <small>一套对象回答 <tt>getString</tt> / <tt>getDrawable</tt> / <tt>loadImage</tt>。</small>"
            "<br/><b><font color='#FF4A90E2'>Looper / Choreographer 原样移植</font></b> —— <small>epoll + eventFd 驱动主线程；无硬件 VSYNC 时自节拍。</small>"
            "<br/><b><font color='#FF4A90E2'>脏区 + blit 合成器</font></b> —— <small>按需重绘，非全屏刷新；像素格式 <tt>ARGB<sub>32</sub></tt>。</small>"
            "<br/><b><font color='#FF4A90E2'>多后端</font></b> —— <small>DRM / fb / SDL / XCB / VNC，一套 GUI 跨平台。</small>");
        set(printerdemo::R::id::about_features,
            "<b>• 50+ 控件 · 20+ Drawable</b> <small>API 兼容 Android</small><br/>"
            "<b>• Fragment + Navigation</b> <small>含 saveState / restoreState</small><br/>"
            "<b>• ConstraintLayout + MotionLayout</b> <small>MotionScene / keyframe</small><br/>"
            "<b>• RecyclerView</b> <small>AndroidX 1:1</small><br/>"
            "<b>• Transition 转场</b> <small>Fade / Slide / ChangeBounds</small><br/>"
            "<b>• 文本栈</b> <small>Spans / StaticLayout / minikin</small><br/>"
            "<b>• 输入</b> <small>KeyCharacterMap / 输入法 / 多点触控</small>");
        set(printerdemo::R::id::about_usecases,
            "<b>• 嵌入式 HMI</b> <small>车机 / 机顶盒 / 工业面板——跑不动完整 Android 时的 Android 级 UI。</small><br/>"
            "<b>• 学习 Android 内部</b> <small>最易读的 AOSP framework 镜像，读 C++ 比啃 AOSP 编译通透。</small><br/>"
            "<b>• C++ 实战</b> <small>真实的所有权 / 生命周期挑战，配现代特性（动画 / 转场 / Material）。</small>"
            "<br/><br/><small>详情与源码：</small> <a href='https://gitee.com/houstudio/Cdroid'>https://gitee.com/houstudio/Cdroid</a>");
    }
};
REGISTER_FRAGMENT(AboutFragment);
