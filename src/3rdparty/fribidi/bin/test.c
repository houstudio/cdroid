#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <fribidi.h>

int main(int argc ,char*argv[]){
    FriBidiCharType base_dir=FRIBIDI_TYPE_ON;
    //FriBidiChar *str=L"بهروز";
    FriBidiChar *str=L" Hello world العالمية!";
    FriBidiChar visstr[128];
    FriBidiStrIndex posLV[18],posVL[128];
    FriBidiLevel  level[128];
    char mbs[256]={0};
    memset(visstr,0,sizeof(visstr));
    int i,len=fribidi_log2vis(str,wcslen(str),&base_dir,visstr,posLV,posVL,level);
    printf("base_dir=%x/%x\r\n",FRIBIDI_TYPE_ON,base_dir); 
    for(i=0;i<wcslen(str);i++)printf("%x ",str[i]);printf("\r\n");
    for(i=0;i<wcslen(str);i++)printf("%x ",visstr[i]);printf("\r\n");
    printf("L2V:");for(i=0;i<wcslen(str);i++)printf("%d,",posLV[i]);printf("\r\n");
    printf("V2L:");for(i=0;i<wcslen(str);i++)printf("%d,",posVL[i]);printf("\r\n");
    printf("LVL:");for(i=0;i<wcslen(str);i++)printf("%d,",level[i]);printf("\r\n");
    return 0;
}
