#ifndef __ZIP_ARCHIVE_H__
#define __ZIP_ARCHIVE_H__
#include <istream>
#include <string>
#include <vector>
#include <memory>
namespace cdroid{

class ZIPArchive{
protected:
  void* zip;
  int method;
public:
  ZIPArchive(const std::string&fname);
  ~ZIPArchive();
  void remove(const std::string&fname)const;
  int getEntries(std::vector<std::string>&entries)const;
  bool hasEntry(const std::string&name,bool excludeDirectories=false)const;
  std::istream* getInputStream(const std::string&fname)const;
  void*getZipHandle(const std::string&fname)const;
};

}

#endif
