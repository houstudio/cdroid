#include <cdtypes.h>
#include <cdlog.h>
#include <stdarg.h> 
#include <ngl_os.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <string>
#include <map>
#include <shared_queue.h>
#include <iomanip>
#include <unistd.h>
#include <execinfo.h>
#include <cxxabi.h>
#if defined(__clang__) || defined(__APPLE__)
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif

static LogLevel sLogLevel=LOG_DEBUG;

static std::string splitFileName(const std::string& str) {
    size_t found;
    found = str.find_last_of("(/\\");
    std::string result=str.substr(found + 1);
    found = result.find_last_of(".");
    return result.substr(0,found); 
}

static std::map<const std::string,int>sModules;
static shared_queue<std::string>dbgMessages;
static char *msgBoddy=nullptr;
static constexpr int kMaxMessageSize=2048;

static void LogInit(){
    static std::once_flag sInit;
    std::call_once(sInit,[&](){
        msgBoddy =new char[kMaxMessageSize];
        std::thread th([](){
            while(1){
                std::string msg;
                dbgMessages.wait_and_pop(msg);
                printf(msg.c_str());
            }
        });th.detach();
    });
}
void LogPrintf(int level,const char*tag,const char*func,int line,const char*format,...){
#if 1
    va_list args;
    const std::string shorttag=splitFileName(tag);
    auto it=sModules.find(tag);
    int module_loglevel=(it==sModules.end())?sLogLevel:it->second;
    struct timespec ts;
    const char*colors[]={"\033[0m","\033[1m","\033[0;32m","\033[0;36m","\033[1;31m","\033[5;31m"};
    if(level<module_loglevel)
        return;
    LogInit();
    if(level<0||level>LOG_FATAL)return;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    int len1=snprintf(msgBoddy,kMaxMessageSize,"%010ld.%06ld \033[0;32m[%s]\033[0;34m \%s:%d %s",
             ts.tv_sec,ts.tv_nsec/1000, shorttag.c_str(),func,line, colors[level]);
    va_start(args, format);
    len1+=vsnprintf(msgBoddy+len1,kMaxMessageSize-len1,format, args);
    va_end(args);
    strcat(msgBoddy+len1,"\033[0m\r\n");
    dbgMessages.push(msgBoddy);
#else
    va_list args;
    cdlog::LogMessage log(tag,line,func,level);
    va_start(args, format);
    log.messageSave(format,args);
    va_end(args);
#endif
}

void LogDump(int level,const char*tag,const char*func,int line,const char*label,const BYTE*data,int len){
    char buff[128];
    int i,j,taglen=0;
    taglen=sprintf(buff,"%s[%d]",label,len);
    cdlog::LogMessage log(tag,line,func,level);
    std::ostringstream &oss=log.messageStream();
    oss<<buff;
    for(int i=0;i<len;i++){
        if(len>32&&i%32==0)oss<<std::endl;
        oss<<' '<<std::hex<<std::setfill('0')<<std::setw(2)<<(int)data[i];
    }
}

void LogSetModuleLevel(const char*module,int level){
    if(module==NULL){
        int i;
        sLogLevel=(LogLevel)level;
        for(auto it=sModules.begin();it!=sModules.end();it++){
            it->second=level;
        }
    }else {
        std::string tag=splitFileName(module);
        sModules[tag]=level;
    }
}

void LogParseModule(const char*log){
   char module[128];
   const char*p=strchr(log,':');
   if(p==NULL)return;
   strncpy(module,log,p-log);
   module[p-log]=0;
   LogLevel l=LOG_DEBUG;
   switch(p[1]){
   case 'v':
   case 'V':l=LOG_VERBOSE;break;
   case 'd':
   case 'D':l=LOG_DEBUG;break;
   case 'i':
   case 'I':l=LOG_INFO;break;
   case 'w':
   case 'W':l=LOG_WARN;break;
   case 'e':
   case 'E':l=LOG_ERROR;break;
   }
   LogSetModuleLevel( ((strcmp(module,"*")==0)?NULL:module),l);
}

void LogParseModules(int argc,const char*argv[]){
   int i;
   for ( i = 1; i <argc; i++) {
       if(strchr(argv[i],':')){
          LogParseModule(argv[i]);
       }
   }
}
#ifdef __cplusplus
namespace cdlog{
static const std::string kTruncatedWarningText = "[...truncated...]";
LogMessage::LogMessage(const std::string& file, const int line, const std::string& function,int level)
   : file_(splitFileName(file)),function_(function) , line_(line),level_(level)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    timestamp_ =ts.tv_sec;
    timeusec_ =ts.tv_nsec/1000 ;
    LogInit();
}

LogMessage::~LogMessage() {
    std::ostringstream oss;
    const char*colors[]={"\033[0m","\033[1m","\033[0;32m","\033[0;36m","\033[1;31m","\033[0;31m"};
    auto it=sModules.find(file_);
    int module_loglevel=(it==sModules.end())?sLogLevel:it->second;

    if(level_>=module_loglevel){
        oss << std::setw(10) <<std::setfill('0')<< (timestamp_) << "." << std::setw(6) << (timeusec_);
        oss << " \033[0;32m[" << splitFileName(file_)<<"]\033[0;34m ";
        oss << function_<<":" << line_ <<" "<<colors[level_];

        const std::string str(stream_.str());
        if (!str.empty()) oss << str ;
        log_entry_ += oss.str();
        log_entry_ +="\033[0m\n";
    
        dbgMessages.push(log_entry_);
    }
}

void LogMessage::messageSave(const char* format, ...){
    va_list arglist;
    char message[kMaxMessageSize];
    va_start(arglist, format);
    const int nbrcharacters = vsnprintf(message, sizeof(message), format, arglist);
    va_end(arglist);
    if (nbrcharacters <= 0) {
        stream_ << '"' << format << '"';
    } else if (nbrcharacters > kMaxMessageSize) {
        stream_  << message << kTruncatedWarningText;
    } else {
        stream_ << message;
    }
}
FatalMessage::FatalMessage(const std::string& file, const int line, const std::string& function,int signal)
 :LogMessage(file,line,function,LOG_FATAL),signal_(signal){
   const size_t max_dump_size = 50;
   void* dump[max_dump_size];
   size_t size = backtrace(dump, max_dump_size);
   char** messages = backtrace_symbols(dump, size); // overwrite sigaction with caller's address

   std::ostringstream oss;
   if(signal_)oss << "Received fatal signal: " << signal_<<std::endl;//g2::internal::signalName(signal_number);
   oss << " PID: " << getpid() << std::endl;

   // dump stack: skip first frame, since that is here
   for (size_t idx = 1; idx < size && messages != nullptr; ++idx) {
       oss<<"\t["<<idx<<"]"<<messages[idx]<<std::endl;
   } // END: for(size_t idx = 1; idx < size && messages != nullptr; ++idx)
   free(messages);
   stream_<<oss.str();
   //std::cout<<log_entry_<<std::endl;
}
FatalMessage::~FatalMessage(){
}
}//endof namespace

#endif

