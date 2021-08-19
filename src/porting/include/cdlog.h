#ifndef __CDLOG_H__
#define __CDLOG_H__

#ifdef __cplusplus
extern "C" {
#endif 

typedef enum{
  LOG_VERBOSE,
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_FATAL
}LogLevel;

void LogPrintf(int level,const char*tag,const char*func,int line,const char*format,...);
void LogDump  (int level,const char*tag,const char*func,int line,const char*label,const BYTE*data,int len);
void LogSetModuleLevel(const char*module,int level);
void LogParseModule(const char*module);
void LogParseModules(int argc,const char*argv[]);
#ifdef __cplusplus
}
#endif 

#ifdef __cplusplus
#include <string>
#include <sstream>
#include <iostream>
#include <cstdarg>
namespace cdlog{
class LogMessage {
protected:
    const std::string file_;
    const int line_;
    const std::string function_;
    int level_;
    std::ostringstream stream_;
    std::string log_entry_;
    long timestamp_; //second part
    long timeusec_;  //usecond part
public:
    LogMessage(const std::string& file, const int line, const std::string& function,int level);
    virtual ~LogMessage(); // at destruction will flush the message
    std::ostringstream& messageStream() {return stream_;}
    void messageSave(const char* format, ...);
};
class FatalMessage:public LogMessage{
protected:
    int signal_;
public:
    FatalMessage(const std::string& file, const int line, const std::string& function,int signal);
    virtual ~FatalMessage();
};
}//namespace cdlog

#define LOG(level)        cdlog::LogMessage(__FILE__,__LINE__,__FUNCTION__,LOG_##level).messageStream()
#define LOG_IF(level,exp) if(exp)cdlog::LogMessage(__FILE__,__LINE__,__FUNCTION__,LOG_##level).messageStream()
#endif //endof __cplusplus

#ifdef __cplusplus
    #define LOG_PRINTF(level,...) cdlog::LogMessage(__FILE__,__LINE__,__FUNCTION__,level).messageSave(__VA_ARGS__)
#else
    #define LOG_PRINTF(level,...) LogPrintf(level,__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#endif

#ifndef DEBUG
    #define LOGV(...)
    #define LOGD(...)
    #define LOGV_IF(x,...)
    #define LOGD_IF(x,...)
    #define LOG_DUMP(tag,data,len)
#else
    #define LOGV(...)  LOG_PRINTF(LOG_VERBOSE,__VA_ARGS__) 
    #define LOGD(...)  LOG_PRINTF(LOG_DEBUG,__VA_ARGS__)
    #define LOGV_IF(x,...)  if(x) LOG_PRINTF(LOG_VERBOSE,__VA_ARGS__) 
    #define LOGD_IF(x,...)  if(x) LOG_PRINTF(LOG_DEBUG,__VA_ARGS__) 
    #define LOG_DUMP(tag,data,len) LogDump(LOG_DEBUG,__FILE__,__FUNCTION__,__LINE__,tag,data,len)
#endif

#define LOGI(...)  LOG_PRINTF(LOG_INFO,__VA_ARGS__) 
#define LOGW(...)  LOG_PRINTF(LOG_WARN,__VA_ARGS__) 
#define LOGE(...)  LOG_PRINTF(LOG_ERROR,__VA_ARGS__) 
#define FATAL(...) cdlog::FatalMessage(__FILE__,__LINE__,__FUNCTION__,0).messageSave(__VA_ARGS__)

#define LOGI_IF(x,...)  if(x) LOG_PRINTF(LOG_INFO,__VA_ARGS__) 
#define LOGW_IF(x,...)  if(x) LOG_PRINTF(LOG_WARN,__VA_ARGS__) 
#define LOGE_IF(x,...)  if(x) LOG_PRINTF(LOG_ERROR,__VA_ARGS__) 
#define FATAL_IF(x,...) if(x) LOG_PRINTF(LOG_FATAL,__VA_ARGS__)
    
#endif//endif __CDLOG_H__
