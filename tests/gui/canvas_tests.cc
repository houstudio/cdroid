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
/*this test used to comfirm all clips is intersected as final result*/
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

TEST_F(CANVAS,rotate1){
   Canvas c(1280,1280);
   c.save();
   c.set_font_size(32);
   c.set_source_rgb(1,0,0);
   c.show_text("Hello world!");
   auto start=SystemClock::uptimeMillis();
   for(int i=0;i<1000;i++){
      c.move_to(100,100);
      c.show_text("Hello world!");
   }
   auto end=SystemClock::uptimeMillis();
   c.restore();
   c.dump2png("rotate0.png");
   printf("used time=%ld\r\n",end-start);

   c.translate(640,640);
   c.rotate_degrees(90);
   c.set_font_size(32);
   c.set_source_rgb(0,1,0);
   start=SystemClock::uptimeMillis();
   for(int i=0;i<1000;i++){
      c.move_to(0,0);
      c.show_text("Hello world!");
   }
   end=SystemClock::uptimeMillis();
   c.dump2png("rotate90.png");
   printf("used time=%ld\r\n",end-start);
}
TEST_F(CANVAS,rotate2){
   Canvas c(1280,1280);
   c.save();
   c.set_font_size(32);
   c.set_source_rgb(1,0,0);
   Cairo::RefPtr<Cairo::ImageSurface>img=Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,100,100);
   Cairo::RefPtr<Cairo::Context>ctx=Cairo::Context::create(img);
   ctx->set_source_rgb(0,0,1);
   ctx->arc(50,50,50,0,M_PI*2.f);
   ctx->fill();
   auto start=SystemClock::uptimeMillis();
   c.set_source(img,0,0);
   for(int i=0;i<1000;i++){
      c.rectangle(0,0,100,100);
      c.fill();
   }
   auto end=SystemClock::uptimeMillis();
   c.restore();
   c.dump2png("img0.png");
   printf("used time=%ld\r\n",end-start);

   c.translate(640,640);
   c.set_source(img,0,0);
   c.rotate_degrees(90);
   c.set_font_size(32);
   c.set_source_rgb(0,1,0);
   start=SystemClock::uptimeMillis();
   for(int i=0;i<1000;i++){
      c.rectangle(0,0,100,100);
      c.fill();
   }
   end=SystemClock::uptimeMillis();
   c.dump2png("img90.png");
   printf("used time=%ld\r\n",end-start);
}

