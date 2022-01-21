#ifndef __FILE_ADAPTER_H__
#define __FILE_ADAPTER_H__
#include <widget/adapter.h>
namespace cdroid{
class FileItem{
public:
    bool isDir;
    size_t fileSize;
    std::string fileName;
    std::string fullpath;
    FileItem();
    FileItem(const FileItem&o);
};

class FileAdapter:public ArrayAdapter<FileItem>{

public:
    View*getView(int position, View* convertView, ViewGroup* parent)override;
    int loadFiles(const std::string&path);
    static std::string SimplifyPath(const std::string & path);
};

}
#endif
