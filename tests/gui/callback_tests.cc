#include <gtest/gtest.h>
#include <windows.h>
#include <ngl_os.h>
#include <sys/time.h>
#include <core/systemclock.h>
#include <cdlog.h>
#include <functional>
#include <callback.h>

class CALLBACK:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(CALLBACK,loop){
   test::CallbackBase<void> c0([](){});
   test::CallbackBase<void,int> c1([](int a){printf("a=%d\r\n",a);});
   test::CallbackBase<void,int,int>cb2([](int a,int b){printf("a=%d,b=%d\r\n",a,b);});
   test::CallbackBase<int,int,int>cbr2([](int a,int b){return a+b;});
   c1(123);
   cb2(123,456);
   printf("cbr2=%d\r\n",cbr2(234,345));
   cbr2=[](int a,int b){return a*b;};
   printf("cbr2=%d\r\n",cbr2(234,345));
}
