#include <string>
#include <vector>
#include <chrono>
#include <zip.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

using namespace std::chrono;

class ZIPArchive{
protected:
  zip_t* zip;
  int method;
  std::string basedir;
public:
  ZIPArchive(const std::string&fname,bool create=false){
     int flags=create?(ZIP_CREATE|ZIP_TRUNCATE):(ZIP_CHECKCONS|ZIP_RDONLY);
     zip=zip_open(fname.c_str(),flags,nullptr);
     method=ZIP_CM_DEFAULT;
  }
  ~ZIPArchive(){
     printf("zip=%p\r\n",zip);
     zip_close(zip);
  }
  void set_compress(int m){
     method=m;
  }
  int add(const std::string&fname){
     struct stat st;
     std::size_t pos=fname.rfind("/");
     if(S_ISDIR(st.st_mode)&&zip_get_num_files(zip)==0){
         basedir=fname;
     }
     if(stat(fname.c_str(),&st)<0)return false;
     if(S_ISDIR(st.st_mode)){
         DIR *dir=opendir(fname.c_str());
         struct dirent *entry;
         while(dir&&(entry=readdir(dir))){
             std::string s=fname;
             if(entry->d_name[0]=='.')
                continue;
             s.append("/");
             s.append(entry->d_name);
             add(s);
         }
         if(dir)closedir(dir);
     }else if(S_ISREG(st.st_mode)){
         std::string name=fname.substr(basedir.length());
         struct zip_source * srcfile=zip_source_file(zip,fname.c_str(),0,-1);
         int rc=zip_file_add(zip,name.c_str(),srcfile,0);//ZIP_FL_OVERWRITE);
         if(rc<0)zip_source_free(srcfile);
         else zip_set_file_compression(zip,rc,method,0);
         printf("[%s]:%d %s\r\n",name.c_str(),S_ISDIR(st.st_mode),fname.c_str());
     }
     return 0;
  }
  bool remove(const std::string&fname){
     zip_name_locate(zip,fname.c_str(),0);
     return true;
  }
  int getEntries(std::vector<std::string>&entries){
      int num=zip_get_num_entries(zip,ZIP_FL_UNCHANGED);
      for(int i=0;i<num;i++){
          const char*name=zip_get_name(zip,i,0);
          entries.push_back(name);
      }
      return num;
  }
  std::istream*getEntryStream(const std::string&fname){
      char buff[128];
      zip_file_t*zfile=zip_fopen(zip,fname.c_str(),ZIP_RDONLY);
      printf("\r\n");
      while(zip_fread(zfile,buff,sizeof(buff))>0)printf("%s",buff);
      zip_fclose(zfile);
      printf("\r\n\r\n");
  }
};

int main(int argc,char*argv[]){
   steady_clock::time_point t2,t1;
  if(argc>=3){
     ZIPArchive za("test.zip",true);
     t1=steady_clock::now();
     za.set_compress(strtoul(argv[1],NULL,10));
     za.add(argv[2]);
  }else{
     t1=steady_clock::now();
     ZIPArchive zr("test.zip");
     std::vector<std::string>entries;
     int num=zr.getEntries(entries);
     for(auto s:entries)printf("%s\r\n",s.c_str());
     if(argc==2)
        zr.getEntryStream(argv[1]);
  }
  t2=steady_clock::now();
  duration<double>dur=duration_cast<duration<double>>(t2 - t1);
  printf("duration=%.5f\r\n",dur);
  return 0;
}
