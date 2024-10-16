#include <stdio.h>
#include <gtest/gtest.h>
#include <porting/dtvos.h>

class OSMEM:public testing::Test{
   public :
   void*p;
   virtual void SetUp(){}
   virtual void TearDown(){}
};

#ifdef ENABLE_DTV

TEST_F(OSMEM,Malloc_1){
   p=nglMalloc(0);
   ASSERT_EQ(NULL,p);
}

TEST_F(OSMEM,Malloc_2){
   p=nglMalloc(100);
   ASSERT_NE(p,(void*)NULL);
   nglFree(p);
}

TEST_F(OSMEM,Alloc){
   p=nglAlloc(0);
   ASSERT_EQ(p,(void*)NULL);
   p=nglAlloc(100);
   ASSERT_NE(p,(void*)NULL);
   for(int i=0;i<100;i++){
      ASSERT_EQ(((unsigned char*)p)[i],0);
   }
   nglFree(p);
}

TEST_F(OSMEM,Realloc_1){
   p=nglMalloc(100);
   ASSERT_TRUE(p);
   ASSERT_TRUE(p=nglRealloc(p,200));
   nglFree(p);
}

TEST_F(OSMEM,Realloc_2){
   p=nglRealloc(NULL,200);
   ASSERT_TRUE(p);
   nglFree(p);
}

TEST(OSMEMDeathTest,Malloc_0){
   int*p=(int*)nglMalloc(0);
}
#endif
