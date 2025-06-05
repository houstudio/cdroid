#include <drawables/vectordrawable.h>
#include <drawables/drawableinflater.h>
#include <core/path.h>
#include <cdroid.h>

// 计算贝塞尔曲线在参数 t 处的分割点
void chop_bezier(const std::vector<PointF>& control_points, double t, std::vector<PointF>& left, std::vector<PointF>& right) {
    size_t n = control_points.size() - 1;
    std::vector<PointF> temp = control_points;

    left.clear();
    right.clear();
    left.push_back(control_points[0]);
    right.push_back(control_points[n]);

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n - i; ++j) {
            temp[j].x = (1.0 - t) * temp[j].x + t * temp[j + 1].x;
            temp[j].y = (1.0 - t) * temp[j].y + t * temp[j + 1].y;
        }
        left.push_back(temp[0]);
        right.push_back(temp[n - i - 1]);
    }
}

// 分割贝塞尔曲线在给定的起始和结束参数 start 和 end 处
void chopAt(const std::vector<PointF>& control_points, double start, double end, std::vector<PointF>& result) {
    std::vector<PointF> left, right, temp;
    // 分割曲线在 start 处
    chop_bezier(control_points, start, left, temp);
    // 分割剩余曲线在 (end - start) / (1.0 - start) 处
    chop_bezier(temp, (end - start) / (1.0 - start), result, right);
    //result;
}

void draw_bezier(Canvas& cr, const std::vector<PointF>& control_points) {
    if (control_points.size() < 2) return;

    cr.move_to(control_points[0].x, control_points[0].y);

    if (control_points.size() == 3) {
        // 二次贝塞尔曲线
        cr.curve_to(control_points[1].x, control_points[1].y, control_points[1].x, control_points[1].y, control_points[2].x, control_points[2].y);
    } else if (control_points.size() == 4) {
        // 三次贝塞尔曲线
        cr.curve_to(control_points[1].x, control_points[1].y, control_points[2].x, control_points[2].y, control_points[3].x, control_points[3].y);
    }
}

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    TextView*tv=new TextView("AnimatedVectorDrawable testcase",0,0);
    w->setBackgroundColor(0xFF112233);
    w->addView(tv);
    Drawable *d = nullptr;
    /*Animator*anim=AnimatorInflater::loadAnimator(&app,"cdroid:anim/btn_radio_to_off_mtrl_ring_outer_animation");
    Animator*anim2=anim->clone();
    LOGD("anim=%p anim2=%d",anim,anim2);
    delete anim;
    delete anim2;*/
    if(argc>1){
        if(strstr(argv[1],".xml"))d=DrawableInflater::loadDrawable(&app,argv[1]);
        else d = new AnimatedImageDrawable(&app,argv[1]);
    }else{
        d = DrawableInflater::loadDrawable(&app,"@cdroid:drawable/btn_check_material_anim");
    }
    LOGD("drawable=%p",d);
    tv->setBackground(d);
    if(dynamic_cast<AnimatedVectorDrawable*>(d)){
        ((AnimatedVectorDrawable*)d)->start();
        d->setLevel(3000);
    }
    if(dynamic_cast<AnimatedImageDrawable*>(d)){
        ((AnimatedImageDrawable*)d)->start();
        LOGD("===webp");
    }
    tv->setOnClickListener([](View&view){
        Drawable *d =view.getBackground();
        static bool checked = false;
        d->setState(checked?StateSet::CHECKED_STATE_SET:StateSet::NOTHING);
        LOGD("checked=%d",checked);
        checked=!checked;
        if(dynamic_cast<AnimatedVectorDrawable*>(d)){
            ((AnimatedVectorDrawable*)d)->start();
        }
    });
    cdroid::Path path;
    cdroid::Canvas c(1024,768);
    std::vector<PointF> control_points = {
        {100, 500},
        {200, 100},
        {600, 100},
        {700, 500} };
    std::vector<PointF> result;
    chopAt(control_points, 0.1, 0.8, result);
    draw_bezier(c,control_points);
    c.set_source_rgb(1,0,0);
    c.stroke();
    //c.translate(0,10);
    draw_bezier(c,result);
    c.set_source_rgb(0,1,0);
    c.stroke();
    c.dump2png("approximate.png");
    app.exec();
    return 0;
}
