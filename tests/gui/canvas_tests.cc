#include <gtest/gtest.h>
#include <cdroid.h>
#include <app/alertdialog.h>
#include <core/canvas.h>

using namespace cdroid;

class CANVAS:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(CANVAS,clip1){
   Canvas c(400,400);
   double x1,y1,x2,y2;
   c.reset_clip();
   c.rectangle(10,5,200,100);
   c.clip();
   c.get_clip_extents(x1,y1,x2,y2);
   printf("clip(%.f,%.f,%.f,%.f)\n",x1,y1,x2,y2);
   ASSERT_DOUBLE_EQ(x1,10);
   ASSERT_DOUBLE_EQ(y1,5);
   ASSERT_DOUBLE_EQ(x2,210);
   ASSERT_DOUBLE_EQ(y2,105);
}

TEST_F(CANVAS,clip2){
   Canvas c(400,400);
   double x1,y1,x2,y2;
   c.reset_clip();
   c.rectangle(10,5,200,100);
   c.rectangle(10,110,200,100);
   c.clip();
   c.get_clip_extents(x1,y1,x2,y2);
   printf("clip(%.f,%.f,%.f,%.f)\n",x1,y1,x2,y2);
   ASSERT_DOUBLE_EQ(x1,10);
   ASSERT_DOUBLE_EQ(y1,5);
   ASSERT_DOUBLE_EQ(x2,210);
   ASSERT_DOUBLE_EQ(y2,210);
   std::vector<Cairo::Rectangle>rects;
   c.copy_clip_rectangle_list(rects);
   for(int i=0;i<rects.size();i++){
      Cairo::Rectangle r=rects.at(i);
      printf("[%d]=(%.f,%.f,%.f,%.f)\r\n",i,r.x,r.y,r.width,r.height);
   }
}
TEST_F(CANVAS,clip3){
   Canvas c(1280,600);
   double x1,y1,x2,y2;
   c.reset_clip();
   c.rectangle(0,0,1280,28);
   c.rectangle(419,28,105,50);
   c.clip();
   c.get_clip_extents(x1,y1,x2,y2);
   printf("clip(%.f,%.f,%.f,%.f)\n",x1,y1,x2,y2);
   ASSERT_DOUBLE_EQ(x1,0);
   ASSERT_DOUBLE_EQ(y1,0);
   ASSERT_DOUBLE_EQ(x2,1280);
   ASSERT_DOUBLE_EQ(y2,78);
   std::vector<Cairo::Rectangle>rects;
   c.copy_clip_rectangle_list(rects);
   for(int i=0;i<rects.size();i++){
      Cairo::Rectangle r=rects.at(i);
      printf("[%d]=(%.f,%.f,%.f,%.f)\r\n",i,r.x,r.y,r.width,r.height);
   }
}

TEST_F(CANVAS,clip4){
   Canvas c(1280,600);
   double x1,y1,x2,y2;
   c.reset_clip();
   c.rectangle(0,0,1280,200);
   c.clip();
   c.rectangle(0,100,1280,100);
   c.clip();
   c.get_clip_extents(x1,y1,x2,y2);
   printf("clip(%.f,%.f,%.f,%.f)\n",x1,y1,x2,y2);
   ASSERT_DOUBLE_EQ(x1,0);
   ASSERT_DOUBLE_EQ(y1,100);
   ASSERT_DOUBLE_EQ(x2,1280);
   ASSERT_DOUBLE_EQ(y2,200);
   std::vector<Cairo::Rectangle>rects;
   c.copy_clip_rectangle_list(rects);
   for(int i=0;i<rects.size();i++){
      Cairo::Rectangle r=rects.at(i);
      printf("[%d]=(%.f,%.f,%.f,%.f)\r\n",i,r.x,r.y,r.width,r.height);
   }
}

