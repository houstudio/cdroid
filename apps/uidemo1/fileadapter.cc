#include <fileadapter.h>
#include <dirent.h>
#include <cdroid.h>
#include <R.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <porting/cdlog.h>
#if defined(_WIN32)||defined(_WIN64)
void realpath(const char* path, char* real) {
    strcpy(real, path);
}
#endif
namespace cdroid{

static int file_filter(const struct dirent*ent){
    return (ent->d_type==DT_REG)||(ent->d_type==DT_DIR);
}

View*FileAdapter::getView(int position, View* convertView, ViewGroup* parent){
    const FileItem& mi=getItemAt(position);
    ViewGroup*vp=(ViewGroup*)convertView;
    TextView*tv;
    if(convertView==nullptr){
        vp=(ViewGroup*)LayoutInflater::from(&App::getInstance())->inflate(mResource,nullptr);
    }
    tv=(TextView*)vp->findViewById(uidemo1::R::id::idno);
    if(tv)tv->setText(std::to_string(position));
    tv=(TextView*)vp->findViewById(uidemo1::R::id::filename);
    tv->setText(mi.fileName);
    tv=(TextView*)vp->findViewById(uidemo1::R::id::filesize);
    if(tv&&mi.isDir==false){
       struct stat st;
       int rc=stat(mi.fullpath.c_str(),&st);
       if(rc==0)
          tv->setText(std::to_string(st.st_size/1024)+"K");
       else tv->setText("stat Error");
    }
    return vp;
}

int FileAdapter::loadFiles(const std::string&filepath){
    std::string path=SimplifyPath(filepath);
    DIR*dir=opendir(filepath.c_str());
    struct dirent*ent;
    int count=0;
    while((ent=readdir(dir))!=nullptr){
        FileItem mi;
        mi.fileName=ent->d_name;
        mi.fullpath=SimplifyPath(path+"/"+mi.fileName);
        //LOGV("%s",mi.fullpath.c_str());
        switch(ent->d_type){
        case DT_DIR:
        case DT_REG:
            mi.isDir=(ent->d_type==DT_DIR);
            add(mi);
            count++;
            break;
        default:break;
        }
    }
    if(dir)closedir(dir);
    //LOGV("%s scaned=%d",filepath.c_str(),count);
    return count;
}

std::string FileAdapter::SimplifyPath(const std::string & path) {
     char* rpath= realpath(path.c_str(),nullptr);
    std::string rs(rpath);
    free(rpath);
    return rs;
}

FileItem::FileItem(){
    fileSize=0;
    isDir=false;
}
FileItem::FileItem(const FileItem&o){
    isDir=o.isDir;
    fileName=o.fileName;
    fullpath=o.fullpath;
}

}
