#include <imagedecoder.h>
#include <fstream>
#include <cdtypes.h>
#include <cdlog.h>
#include <framesequence.h>
#include <gui/cdroid.h>
#include <core/app.h>
#include <utils/textutils.h>
#include <image-decoders/framesequence.h>
#include <dirent.h>
#include <sys/stat.h>
static void scan_dir(const char*dir);
int main(int argc,const char*argv[]){
#if 10
    scan_dir(argv[1]);
    return 0;
    auto image=ImageDecoder::loadImage(nullptr,argv[1]);
    image->write_to_png("111.png");
#else
    std::ifstream file(argv[1]);
    cdroid::FrameSequence*seq=cdroid::FrameSequence::create(nullptr,argv[1]);
    LOGD("%dx%dx%d",seq->getWidth(),seq->getHeight(),seq->getFrameCount());
    Cairo::RefPtr<Cairo::ImageSurface>img=Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,seq->getWidth(),seq->getHeight());
    cdroid::FrameSequenceState*state=seq->createState();
    for(int i=0;i<seq->getFrameCount();i++){
        state->drawFrame(i,(uint32_t*)img->get_data(),img->get_stride()/4,0);
        img->write_to_png(std::to_string(i)+std::string(".png"));
    }
    //seq->createState()->drawFrame(0,(uint32_t*)img->get_data(),img->get_stride()/4,0);
    /*Window*w=new Window(0,0,-1,-1);
    AnimatedImageDrawable*ad=new AnimatedImageDrawable(&app,app.getParam(0,"./Honeycam1.gif"));
    cdroid::TextView*tv=new cdroid::TextView("Hello World!",100,100);
    w->addView(tv);
    tv->setBackground(ad);
    ad->start();*/
#endif
    return 0;
}

static void scan_dir(const char*dir){
    struct dirent *entry;
    struct stat entry_stat;
    DIR *dp = opendir(dir);

    if (dp == NULL) {
        const char* tnms[] = {"NULL","TRANSLUCENT","TRANSPARENT","OPAQUE"};
        auto image= ImageDecoder::loadImage(nullptr,dir);
        int trans = ImageDecoder::getTransparency(image);
        printf("image[%p] trans=%d/%d %12s path=%s\r\n",(image?image.get():nullptr),
                trans,ImageDecoder::computeTransparency(image),tnms[trans],dir);
        return;
    }

    printf("Directory: %s\n", dir);

    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;  // 跳过当前和上级目录
        }

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        // 获取文件状态
        if (stat(path, &entry_stat) == -1) {
            perror("stat");
            continue;
        }

        // 判断文件类型
        if (S_ISDIR(entry_stat.st_mode)) {
            // 递归扫描子目录
            //scan_dir(path);
        } else {
            const char* tnms[] = {"NULL","TRANSLUCENT","TRANSPARENT","OPAQUE"};
            auto image= ImageDecoder::loadImage(nullptr,path);
            const int t1 = ImageDecoder::getTransparency(image);
            const int t2 = ImageDecoder::computeTransparency(image);
            if(image==nullptr)continue;
            printf("image[%p] %4dx%-4d trans=%d/%d %12s [%s]\r\n",image.get(),
                    image->get_width(),image->get_height(), t1,t2,tnms[t2],entry->d_name);
        }
    }
    closedir(dp);
}
