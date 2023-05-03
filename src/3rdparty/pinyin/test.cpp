#include<pinyinime.h>
#include<utf16char.h>
#include<stdio.h>
#include<strings.h>

using namespace ime_pinyin;
#define kMaxPredictNum 32
static char16 (*predict_buf)[kMaxPredictSize + 1];

int  main(int argc,char*argv[]){
   void*hime=im_open_decoder(argv[1],"userdict");
   char16 hou[]={'n','i','n','h','a','o',0};
   char16 *predicts[kMaxPredictSize+1];
   char16 str[kMaxPredictSize+1];

   size_t cannum=im_search(hime,"ninhao",6);
   printf("cannum=%d\r\n",cannum); 
   for(int i=0;i<cannum;i++){/*拼音转汉字*/
      char16 canbuf[32];
      char16*scan=im_get_candidate(hime,i,canbuf,32);
      printf("len=%d ",utf16_strlen(scan));
      for(int j=0;j<utf16_strlen(scan);j++)printf("%04x ",scan[j]);
      printf("\r\n");
   }
   utf16_strncpy(str,hou,3);str[3]=0;
   size_t predict_len=im_get_predicts(hime,str,predict_buf);
   for(int i=0;i<predict_len;i++){/*联想预测*/
        int len=utf16_strlen(predict_buf[i]);
        printf("predict[%d] len=%d ",i,len);
        for(int j=0;j<len;j++)printf("%04x ",predict_buf[i][j]);
        printf("[%s]\r\n",predict_buf[i]);
   }
   return 0;
}
