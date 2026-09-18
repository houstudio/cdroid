#include <drawable/vectordrawable.h>
#include <drawable/drawableinflater.h>
#include <core/path.h>
#include <core/xmlpullparser.h>
#include <widget/internal_R.h>
#include <cdroid.h>
#include <fstream>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    TextView*tv=new TextView(&app); tv->setText("AnimatedVectorDrawable" );
    w->setBackgroundColor(0xFF112233);
    w->addView(tv);
    Drawable *d = nullptr;
    if(argc>1){
        // A file on disk has no resource id — parse it and inflate from the
        // root tag (resource drawables go through App.getDrawable(int)).
        if(strstr(argv[1],".xml")){
            auto parser = XmlPullParser::detectAndCreate(&app,
                    std::make_unique<std::ifstream>(argv[1]));
            int type;
            while(((type=parser->next())!=XmlPullParser::START_TAG)
                    &&(type!=XmlPullParser::END_DOCUMENT)){}
            if(type==XmlPullParser::START_TAG){
                const AttributeSet& attrs = *parser;
                d = DrawableInflater::inflateFromXml(app.getResources(),
                        parser->getName(),*parser,attrs);
            }
        }
    }else{
        d = app.getDrawable(cdroid::internal::R::drawable::btn_check_material_anim);
    }
    LOGD("drawable=%p",d);
    tv->setBackground(d);
    if(dynamic_cast<AnimatedVectorDrawable*>(d)){
        //d->setLevel(100);
        LOGD("AnimatedVectorDrawable");
        ((AnimatedVectorDrawable*)d)->start();
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
    app.exec();
    return 0;
}
