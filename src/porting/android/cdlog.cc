#include <cdlog.h>
#include <cstdio>
#include <string.h>
#include <time.h>
#include <string>
#include <map>
#include <shared_queue.h>
#include <iomanip>
#include <android/log.h>
#if defined(__Linux__)||defined(__unix__)
#include <unistd.h>
#include <cxxabi.h>
#if defined(__clang__) || defined(__APPLE__)
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif
#endif
#include <limits.h>
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif
static LogLevel sLogLevel=LOG_DEBUG;

static std::string splitFileName(const std::string& str) {
    size_t found;
    std::string result=str;
    found = str.find_last_of("(/\\");
    if(found!=std::string::npos)
        result=str.substr(found + 1);
    found = result.find_last_of(".");
    if(found!=std::string::npos)
        return result.substr(0,found);
    return result;
}

static std::map<const std::string,int>sModules;
static shared_queue<std::string>dbgMessages;
static constexpr int kMaxMessageSize=2048;
static char msgBoddy[kMaxMessageSize];
static int cdlog2a(int level){
    switch(level){
    /** Verbose logging. Should typically be disabled for a release apk. */
    case LOG_VERBOSE:return ANDROID_LOG_VERBOSE; /** Verbose logging. Should typically be disabled for a release apk. */
    case LOG_DEBUG: return ANDROID_LOG_DEBUG; /** Debug logging. Should typically be disabled for a release apk. */
    case LOG_INFO : return ANDROID_LOG_INFO;  /** Informational logging. Should typically be disabled for a release apk. */
    case LOG_WARN : return ANDROID_LOG_WARN;  /** Warning logging. For use with recoverable failures. */
    case LOG_ERROR: return ANDROID_LOG_ERROR; /** Error logging. For use with unrecoverable failures. */
    case LOG_FATAL: return ANDROID_LOG_FATAL; /** Fatal logging. For use when aborting. */
    default :return ANDROID_LOG_SILENT;  /** For internal use only.  */
    }
    return ANDROID_LOG_UNKNOWN;
}

static void LogInit() {
}

void LogPrintf(int level,const char*file,const char*func,int line,const char*format,...) {
    va_list args;
    const std::string tag=splitFileName(file);
    auto it = sModules.find(tag);
    const int module_loglevel=(it==sModules.end())?sLogLevel:it->second;
    if(level<module_loglevel||level<0||level>LOG_FATAL)
        return;
    LogInit();
    const char*colors[]= {"\033[0m","\033[1m","\033[0;32m","\033[0;36m","\033[1;31m","\033[5;31m"};
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    int len1=snprintf(msgBoddy,kMaxMessageSize,"%010ld.%06ld \033[0;32m[%s]\033[0;34m \%s:%d %s",
                      ts.tv_sec,ts.tv_nsec/1000, tag.c_str(),func,line, colors[level]);
    va_start(args, format);
    len1+=vsnprintf(msgBoddy+len1,kMaxMessageSize-len1,format, args);
    va_end(args);
    strcat(msgBoddy+len1,"\033[0m\r\n");
    __android_log_print(cdlog2a(level),file,"%s",msgBoddy);
}

void LogDump(int level,const char*tag,const char*func,int line,const char*label,const unsigned char*data,int len) {
    char buff[128];
    int i,taglen=0;
    taglen = sprintf(buff,"%s[%d]",label,len);
    cdlog::LogMessage log(tag,line,func,level);
    std::ostringstream &oss=log.messageStream();
    oss<<buff;
    for(int i=0; i<len; i++) {
        if(len>32&&i%32==0)oss<<std::endl;
        oss<<' '<<std::hex<<std::setfill('0')<<std::setw(2)<<(int)data[i];
    }
}

void LogSetModuleLevel(const char*module,int level) {
    if(module==NULL) {
        sLogLevel=(LogLevel)level;
        for(auto it=sModules.begin(); it!=sModules.end(); it++) {
            it->second=level;
        }
    } else {
        std::string tag=splitFileName(module);
        sModules[tag]=level;
    }
}

void LogParseModule(const char*log) {
    char module[128];
    const char*p = strchr(log,':');
    if(p==NULL)return;
    strncpy(module,log,p-log);
    module[p-log]=0;
    LogLevel l=LOG_DEBUG;
    switch(p[1]) {
    case 'v':
    case 'V':
        l=LOG_VERBOSE;
        break;
    case 'd':
    case 'D':
        l=LOG_DEBUG;
        break;
    case 'i':
    case 'I':
        l=LOG_INFO;
        break;
    case 'w':
    case 'W':
        l=LOG_WARN;
        break;
    case 'e':
    case 'E':
        l=LOG_ERROR;
        break;
    }
    LogSetModuleLevel( ((strcmp(module,"*")==0)?NULL:module),l);
}

void LogParseModules(int argc,const char*argv[]) {
    for (int i = 1; i <argc; i++) {
        if(strchr(argv[i],':')) {
            LogParseModule(argv[i]);
        }
    }
}


namespace cdlog {
static const std::string kTruncatedWarningText = "[...truncated...]";
LogMessage::LogMessage(const std::string& file, const int line, const std::string& function,int level)
    : file_(splitFileName(file)),function_(function), line_(line),level_message(level) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    timestamp_ = ts.tv_sec;
    timeusec_ = ts.tv_nsec/1000 ;
    LogInit();
    const std::string tag=splitFileName(file);
    auto it = sModules.find(tag);
    level_module=(it==sModules.end())?sLogLevel:it->second;
}

LogMessage::~LogMessage() {
    std::ostringstream oss;
    static const char*colors[]= {"\033[0m","\033[1m","\033[0;32m","\033[0;36m","\033[1;31m","\033[0;31m"};

    if(level_message>=level_module) {
        oss << std::setw(10) <<std::setfill('0')<< (timestamp_) << "." << std::setw(6) << (timeusec_);
        oss << " \033[0;32m[" << file_<<"]\033[0;34m ";
        oss << function_<<":" << line_ <<" "<<colors[level_message];

        const std::string str(stream_.str());
        if (!str.empty()) oss << str ;
        log_entry_ += oss.str();
        log_entry_ +="\033[0m\n";
        __android_log_print(cdlog2a(level_message),file_.c_str(),"%s",log_entry_.c_str());
    }
}

void LogMessage::messageSave(const char* format, ...) {
    if(level_message>=level_module) {
        va_list arglist;
        char message[kMaxMessageSize]={0};
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
}
FatalMessage::FatalMessage(const std::string& file, const int line, const std::string& function,int signal)
    :LogMessage(file,line,function,LOG_FATAL),signal_(signal) {
    const size_t max_dump_size = 50;
    void* dump[max_dump_size];
#ifdef HAVE_EXECINFO_H
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
#endif
    //std::cout<<log_entry_<<std::endl;
}
FatalMessage::~FatalMessage() {
}
}//endof namespace

