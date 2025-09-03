#ifndef __GUI_ENVIRONMENT_H__
#define __GUI_ENVIRONMENT_H__
#include <gtest/gtest.h>
#include <cdroid.h>
#include <signal.h>
#include <cdlog.h>

class GUIEnvironment: public testing::Environment{
private:
    int argc;
    const char**argv;
    static GUIEnvironment*mInst;
public:
    GUIEnvironment(int c,const char*v[]):argc(c),argv(v){
        mInst=this;
    }
    void SetUp(){
       printf("GUIEnvironment Setup\r\n");
    }
    void TearDown(){
       printf("GUIEnvironment TearDown\r\n");
    }
    int getArgc()const{
        return argc;
    }
    const char**getArgv()const{
        return argv;
    }
    static GUIEnvironment*getInstance(){
        return mInst;
    }
};
#endif/*__GUI_ENVIRONMENT_H__*/
