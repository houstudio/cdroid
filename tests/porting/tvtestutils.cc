#include <tvtestutils.h>
#include <gtest/gtest.h>
#include <gtest/internal/gtest-port.h>

namespace tvutils{

//testing::internal::StringFromGTestEnv("tunning","C:685000,6875,64");
//testing::internal::StringFromGTestEnv("tunning","S:685000,6875,hor,22k");
//testing::internal::StringFromGTestEnv("tunning","T:685000,8");
int GetTuningParams(NGLTunerParam&tp,int*lnb,int*_22k){
    const char*params=testing::internal::StringFromGTestEnv("tunning","C:685000,6875,64");
    const char*p=strpbrk(params,"CST");
    static const char *qamnames[]={"Unkown","QAM16","QAM32","QAM64","QAM128","QAM256","QAM512","QAM1024"};
    static const char *polarnames[]={"Horz","Vert","Left","Right"};
    int delivery;
    int ilnb=0,i22k=0;
    if(p==NULL)return 0;
    delivery=*p;
    p=strchr(p,':');
    if(p==NULL)return 0;
    switch(delivery){
    case 'C':tp.delivery_type=DELIVERY_C;
             tp.frequency=atoi(p+1);
             p=strpbrk(p+1,",;");
             tp.u.c.symbol_rate=atoi(p+1);
             p=strpbrk(p+1,",;");
             for(int qm=16,i=4;i<=8;i++,qm*=2){
                if(atoi(p+1)==qm){ 
                    tp.u.c.modulation=(NGLQamMode)(i-3);
                    break;
                }
             }
             printf("\033[5;31mDVBC:%d,%d,%s \033[0m\r\n",tp.frequency,tp.u.c.symbol_rate,qamnames[tp.u.c.modulation]);
             break;
    case 'S':tp.delivery_type=DELIVERY_S;
             tp.frequency=atoi(p+1);
             p=strpbrk(p+1,",;");
             tp.u.s.symbol_rate=atoi(p+1);
             p=strpbrk(p+1,",;");
             ilnb=p?(NGLNimPolar)atoi(p+1):0;
             tp.u.s.polar=(NGLNimPolar)ilnb;
             if(p)i22k=atoi(p+1);
             printf("\033[5;31mDVBS:%d,%d %s 22k[%s]\033[0m\r\n",tp.frequency, tp.u.s.symbol_rate,
                 polarnames[ilnb],(i22k?"ON":"OFF")); 
             break;
    case 'T':tp.delivery_type=DELIVERY_T;
             tp.frequency=atoi(p+1);
             p=strpbrk(p+1,",;");
             tp.u.t.bandwidth=(NGLBandWidth)(atoi(p+1)-5);
             printf("\033[5;31mDVBT:%d,BANDWIDTH_%dM \033[0m",tp.frequency,tp.u.t.bandwidth+5);
             break;
    default:return 0;
    }
    return delivery;
}

}


