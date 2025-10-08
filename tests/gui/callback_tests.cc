#include <gtest/gtest.h>
#include <cdroid.h>
#include <core/systemclock.h>
#include <cdlog.h>
#include <functional>
#include <core/callbackbase.h>

class CALLBACK:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(CALLBACK,loop){
   CallbackBase<void> c0([](){});
   CallbackBase<void,int> c1([](int a){printf("a=%d\r\n",a);});
   CallbackBase<void,int,int>cb2([](int a,int b){printf("a=%d,b=%d\r\n",a,b);});
   CallbackBase<int,int,int>cbr2([](int a,int b){return a+b;});
   c1(123);
   cb2(123,456);
   printf("cbr2=%d\r\n",cbr2(234,345));
   cbr2=[](int a,int b){return a*b;};
   printf("cbr2=%d\r\n",cbr2(234,345));
}
TEST_F(CALLBACK,NUL){
    CallbackBase<void>n1(nullptr);
    CallbackBase<void>n2=nullptr;
    CallbackBase<int,int>n11(nullptr);
    CallbackBase<int,int>n22(nullptr);
    ASSERT_TRUE(n1==nullptr);
    ASSERT_TRUE(n2==nullptr);
    ASSERT_FALSE(n1!=nullptr);
    ASSERT_FALSE(n2!=nullptr);
    ASSERT_TRUE(n11==nullptr);
    ASSERT_TRUE(n22==nullptr);
    ASSERT_FALSE(n11!=nullptr);
    ASSERT_FALSE(n22!=nullptr);

    n1=n2;
    ASSERT_TRUE(n1==n2);
    n11=n22;
    ASSERT_TRUE(n11==n22);
}

TEST_F(CALLBACK,eq){
   CallbackBase<void>c0([](){});
   CallbackBase<void>c1(c0);
   CallbackBase<void>c2([](){});
   Runnable r1=[](){};
   Runnable r2=r1;
   ASSERT_TRUE(c0==c1);
   ASSERT_TRUE(r1==r2);

   ASSERT_FALSE(c0==c2);
   ASSERT_FALSE(c1==c2);

   c2=c1;
   ASSERT_TRUE(c2==c1);
   ASSERT_TRUE(c2==c0);
}

TEST_F(CALLBACK,runner){
    Runnable r;
    ASSERT_TRUE(r==nullptr);
    r=[](){};
    ASSERT_FALSE(r==nullptr);
}

class EventA:public virtual EventSet{};
class EventB:public virtual EventSet{};
class EventC:public virtual EventA,virtual EventB{
};
TEST_F(CALLBACK,EventSet){
    EventSet a;
    EventSet b=a;
    ASSERT_TRUE(a==b);
    ASSERT_TRUE(b==a);
    ASSERT_FALSE(a!=b);
    ASSERT_FALSE(b!=a);
    EventC cc;
    EventSet aa=cc;
    EventSet bb=cc;
    ASSERT_TRUE(cc==aa);
    ASSERT_TRUE(cc==bb);
    ASSERT_TRUE(aa==bb);
}
class MyListener:public cdroid::EventSet{
public:
   CallbackBase<void>onInit;
   CallbackBase<void>onDestroy;
};

TEST_F(CALLBACK,EventSetEqual){
    MyListener ls,ls2;
    ls.onInit=[](){LOGD("onInit");};
    ls.onDestroy=[](){LOGD("onDestroy");};
    ls2=ls;
    ASSERT_EQ(ls2,ls);
    ASSERT_EQ(ls2.onInit,ls.onInit);
    ASSERT_EQ(ls2.onDestroy,ls.onDestroy);
}

class MyRunner:public Runnable{
private:
    int mData;
public:
    MyRunner(){mData =0;};
    void operator()()override{
        LOGD("mData=%d",mData++);
    }
};
TEST_F(CALLBACK,inherited){
    MyRunner run;
    run();
    run();
}
