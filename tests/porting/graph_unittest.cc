#include<cdgraph.h>
#include<cdlog.h>
#include <gtest/gtest.h>
#include <sys/time.h>

#define INVALID_COLOR 0x01010101
typedef struct{
   int fmt;
   uint8_t ashift;
   uint8_t rshift;
   uint8_t gshift;
   uint8_t bshift;
   uint8_t amask;
   uint8_t rmask;
   uint8_t gmask;
   uint8_t bmask;
   uint8_t aloss;
   uint8_t rloss;
   uint8_t gloss;
   uint8_t bloss;
}FORMAT;

FORMAT formats[]={
    {GPF_UNKNOWN,0,0,0,0,  0,0,0,0},
    {GPF_ARGB4444,12, 8,4, 0,0xFF,0xFF,0xFF,0xFF,0,0,0,0},
    {GPF_ARGB1555,15,10,5, 0,0x80,0x7C,0x3E,0x1F,0,0,0,0},
    {GPF_ARGB,    24,16,8, 0,0xFF,0xFF,0xFF,0xFF,0,0,0,0},
    {GPF_ABGR,    24,0 ,8,16,0xFF,0xFF,0xFF,0xFF,0,0,0,0}
};

class GRAPH:public testing::Test{
public :
    static void SetUpTestCase(){
       GFXInit();
       for(int f=GPF_ARGB4444;f<=GPF_ABGR;f++){
          printf("format %d whitepixel=%x\r\n",f,Color2Pixel(f,0xFFFFFFFF));
          printf("format %d whitealphapixel=%x\r\n",f,Color2Pixel(f,0x00FFFFFF));
       }
    }
    static void TearDownTestCase(){
       sleep(20);
    }
    virtual void SetUp(){
    }
    static uint32_t Color2Pixel(int32_t fmt,uint32_t color){
        FORMAT*f = formats+fmt;
        uint8_t a,r,g,b;
        uint32_t pixel;
        a = color>>24;
        r = color>>16;
        g = color>>8;
        b = color;
        pixel = ((a&f->amask)<<f->ashift)|((r&f->rmask)<<f->rshift)|((g&f->gmask)<<f->gshift)|((b&f->bmask)<<f->bshift);
        return pixel;
    }
    virtual void TearDown(){
    }
    unsigned long long gettime(){
        struct timeval tv;
        gettimeofday(&tv,NULL);
        return tv.tv_sec*1000+tv.tv_usec/1000;
    }
    unsigned int getPixel(HANDLE surface,int x,int y){
        //for pixel(x,y)out of surface,we return INVALID_COLOR
        uint8_t*buffer;
        uint32_t w,h,f,pitch;
        GFXLockSurface(surface,(void**)&buffer,&pitch);
        GFXGetSurfaceInfo(surface,&w,&h,(int*)&f);
        if(x<0||y<0||x>=w||y>=h)return INVALID_COLOR;
        buffer+=pitch*y;
        switch(f){
        case GPF_ARGB4444:
        case GPF_ARGB1555:return *(uint16_t*)(buffer+2*x);
        case GPF_ARGB:
        case GPF_ABGR:
        case GPF_RGB32:return *(uint32_t*)(buffer+4*x);
        default:return 0xCCCCCCCC;
       }
   }
   BOOL errorPixel(HANDLE surface,int x,int y,uint32_t color){
   //check color of pixel(x,y) =color
       uint32_t cc=getPixel(surface,x,y);
       return ((cc!=color)&&(cc!=INVALID_COLOR))||(cc==INVALID_COLOR);
   }
   void setRect(GFXRect&r,int x,int y,uint32_t w,uint32_t h){
       r.x=x;r.y=y;r.w=w;r.h=h;
   }
   int CheckColor(HANDLE surface,GFXRect*rec,uint32_t c){
       int rc=0;
       uint32_t w,h,f;
       GFXRect r;
       GFXGetSurfaceInfo(surface,&w,&h,(int*)&f); 
       if(rec)
          r=*rec;
       else 
          setRect(r,0,0,w,h);

       //check color of rect(inner)
       if(c==getPixel(surface,r.x,r.y))rc|=1;            //topleft
       if(c==getPixel(surface,r.x+(int)r.w-1,r.y))rc|=2;      //topright
       if(c==getPixel(surface,r.x+(int)r.w-1,r.y+(int)r.h-1))rc|=4;//rightbottom
       if(c==getPixel(surface,r.x,r.y+(int)r.h-1))rc|=8;      //leftbottom
       
       //check color outof rect(outer)
       if(errorPixel(surface,r.x  ,r.y-1,c))rc|=0x10;       
       if(errorPixel(surface,r.x-1,r.y-1,c))rc|=0x20;       
       if(errorPixel(surface,r.x-1,r.y+1,c))rc|=0x40;       

       if(errorPixel(surface,r.x+(int)r.w-1 ,r.y-1,c))rc|=0x80;
       if(errorPixel(surface,r.x+(int)r.w   ,r.y-1,c))rc|=0x100;
       if(errorPixel(surface,r.x+(int)r.w   ,r.y,c)||1)rc|=0x200;

       if(errorPixel(surface,r.x+(int)r.w   ,r.y+(int)r.h-1,c))rc|=0x400;
       if(errorPixel(surface,r.x+(int)r.w   ,r.y+r.h,c))rc|=0x800;
       if(errorPixel(surface,r.x+(int)r.w-1 ,r.y+r.h,c))rc|=0x1000;

       if(errorPixel(surface,r.x-1,r.y+(int)r.h-1,c))rc|=0x2000;
       if(errorPixel(surface,r.x-1,r.y+(int)r.h,c))rc|=0x4000;
       if(errorPixel(surface,r.x,r.y+(int)r.h,c))rc|=0x8000;
       return rc;
   }
   int FillCheckColor(HANDLE surface,GFXRect*rec,uint32_t c){
       GFXFillRect(surface,rec,c);
       return CheckColor(surface,rec,c);
   }
};

TEST_F(GRAPH,Graph_GetScreen){
    uint32_t w,h;
    ASSERT_EQ(0,GFXGetDisplaySize(0,&w,&h));
    ASSERT_GT(w,0);
    ASSERT_GT(h,0);
}

TEST_F(GRAPH,CreateSurface_1){
    HANDLE surface=0;
    uint32_t width,height;
    ASSERT_EQ(0,GFXGetDisplaySize(0,&width,&height));
    ASSERT_EQ(0,GFXCreateSurface(0,&surface,width,height,GPF_ARGB,1));
    ASSERT_NE((HANDLE)nullptr,surface);

    GFXRect r={100,100,400,400};
    ASSERT_EQ(0,GFXFillRect(surface,&r,0xFFFF00FF));
    ASSERT_EQ(0,GFXFlip(surface));
    ASSERT_NE((HANDLE)nullptr,surface);
    sleep(2);
    ASSERT_EQ(0,GFXDestroySurface(surface));
}

TEST_F(GRAPH,CreateSurface_2){
    uint32_t width,height,pitch;
    int fmts[]={GPF_ARGB4444,GPF_ARGB1555,GPF_ARGB,GPF_ABGR,GPF_RGB32};
    int bps[] ={         2,         2    ,  4     ,    4   ,    4    }; 
    ASSERT_EQ(0,GFXGetDisplaySize(0,&width,&height));
    ASSERT_GT(width*height,0);
    for(int i=0;i<sizeof(fmts)/sizeof(int);i++){
        uint32_t w=0,h=0,fmt=0;
        HANDLE surface=NULL;
        void*buffer=NULL;
        int32_t  rc=GFXCreateSurface(0,&surface,width,height,fmts[i],0);
        ASSERT_TRUE((rc==E_NOT_SUPPORT)||(rc==E_OK));
        if(rc==E_NOT_SUPPORT)
            continue;
        ASSERT_NE((HANDLE)nullptr,surface);
        ASSERT_EQ(0,GFXGetSurfaceInfo(surface,&w,&h,(int*)&fmt));
        ASSERT_GT(w*h,0);
        ASSERT_EQ(fmt,fmts[i]);
        GFXLockSurface(surface,(void**)&buffer,&pitch);
        ASSERT_EQ(0,GFXUnlockSurface(surface));
        ASSERT_NE((void*)nullptr,buffer);
        ASSERT_GE(pitch,width*bps[i]);
        ASSERT_LE(pitch,width*bps[i]+128);
        ASSERT_EQ(0,GFXDestroySurface(surface));
    }
}

TEST_F(GRAPH,Alpha){
    HANDLE surface=0;
    uint32_t width,height,pitch;
    uint32_t *buffer;
    ASSERT_EQ(0,GFXGetDisplaySize(0,&width,&height));
    ASSERT_EQ(0,GFXCreateSurface(0,&surface,width,height,GPF_ARGB,1));
    GFXLockSurface(surface,(void**)&buffer,&pitch);
    GFXFillRect(surface,NULL,0xFFFFFFFF);
    for(int i=0;i<256;i++){
        GFXSurfaceSetOpacity(surface,i);
        for(int j=0;j<1000;j++)
            ASSERT_EQ(buffer[j],0xFFFFFFFF);
    }
    ASSERT_EQ(0,GFXDestroySurface(surface));     
}

TEST_F(GRAPH,Colors){
    HANDLE surface=0;
    uint32_t width,height;
    GFXRect r={0,0,0,0};
    ASSERT_EQ(0,GFXGetDisplaySize(0,&width,&height));
    ASSERT_EQ(0,GFXCreateSurface(0,&surface,width,height,GPF_ARGB,1));
    r.w=width/2;r.h=height/2;
    GFXFillRect(surface,&r,0xFFFF0000);

    r.x+=width/2;
    GFXFillRect(surface,&r,0xFF00FF00);

    r.y+=height/2;
    GFXFillRect(surface,&r,0xFF0000FF);
    r.x=0;
    GFXFillRect(surface,&r,0xFFFFFFFF);
    sleep(2); 
    ASSERT_EQ(0,GFXDestroySurface(surface));
}

TEST_F(GRAPH,Blit){
    HANDLE mainsurface=0,surface;
    uint32_t width,height;
    GFXRect r={0,0,0,0};
    ASSERT_EQ(0,GFXGetDisplaySize(0,&width,&height));
    ASSERT_EQ(0,GFXCreateSurface(0,&mainsurface,width,height,GPF_ARGB,1));
    ASSERT_EQ(0,GFXCreateSurface(0,&surface,width,height,GPF_ARGB,0));
    r.w=width/2;r.h=height/2;
    GFXFillRect(surface,&r,0xFFFF0000);

    r.x+=width/2;
    GFXFillRect(surface,&r,0xFF00FF00);

    r.y+=height/2;
    GFXFillRect(surface,&r,0xFF0000FF);
    r.x=0;
    GFXFillRect(surface,&r,0xFFFFFFFF);
    GFXBlit(mainsurface,0,0,surface,NULL);
    sleep(2);
    ASSERT_EQ(0,GFXDestroySurface(surface));
    ASSERT_EQ(0,GFXDestroySurface(mainsurface));
}

TEST_F(GRAPH,Multilayer){
    HANDLE hwsurface;
    uint32_t width,height;
    HANDLE layers[4]={NULL,NULL,NULL,NULL};
    struct timeval tv;
    ASSERT_EQ(0,GFXGetDisplaySize(0,&width,&height));
    ASSERT_EQ(0,GFXCreateSurface(0,&hwsurface,width,height,GPF_ARGB,1));
    
    for(int i=0;i<4;i++){
        GFXCreateSurface(0,&layers[i],width-i*100,height-i*50,GPF_ARGB,0);
    }
    for(int k=0;k<1500;k++){
        GFXFillRect(hwsurface,NULL,0);
        for(int i=0;i<4;i++){
           gettimeofday(&tv,NULL);
           if(layers[i])GFXFillRect(layers[i],NULL,0xFF000000|(tv.tv_usec+(i*i*i)<<(i*2)));
        }
        for(int i=0;i<4;i++){
           int sw,sh,fmt;
           gettimeofday(&tv,NULL);
           srandom(tv.tv_usec);
           if(layers[i]){
               int x=k-500+random()%width;
               int y=k-500+random()%height;
               GFXGetSurfaceInfo(layers[i],(uint32_t*)&sw,(uint32_t*)&sh,&fmt);
               if((x+sw>0) && (y+sh>0) && (x<(int)width) && (y<(int)height)) 
                   ASSERT_EQ(0,GFXBlit(hwsurface,x,y,layers[i],NULL));
               else
                   ASSERT_NE(0,GFXBlit(hwsurface,x,y,layers[i],NULL));
           }
        }
        GFXFlip(hwsurface); 
    }
    for(int i=0;i<4;i++)
       GFXDestroySurface(layers[i]);
    GFXDestroySurface(hwsurface); 
}

TEST_F(GRAPH,FillRect){
    HANDLE surface=0;
    GFXRect r;
    uint32_t width,height;
    ASSERT_EQ(0,GFXGetDisplaySize(0,&width,&height));
    ASSERT_EQ(0,GFXCreateSurface(0,&surface,width,height,GPF_ARGB,1));
    ASSERT_NE((HANDLE)nullptr,surface);

    ASSERT_EQ(0xFFFF,FillCheckColor(surface,NULL,0));

    setRect(r,0,0,width,height);
    ASSERT_EQ(0xFFFF,FillCheckColor(surface,&r,0xFF00000));
    

    setRect(r,1,1,width-2,height-2);
    ASSERT_EQ(0xFFFF,FillCheckColor(surface,&r,0xFF00FF00));

    setRect(r,10,10,width-200,height-200);
    ASSERT_EQ(0xFFFF,FillCheckColor(surface,&r,0xFF00FFFF));
    setRect(r,10,10,width-200,height-200);
    ASSERT_EQ(0xFFFF,FillCheckColor(surface,&r,0x00000000));
    GFXFlip(surface);
    ASSERT_EQ(0,GFXDestroySurface(surface));
}
#define TEST_TIMES 1000
TEST_F(GRAPH,Benchmark_Fill){
   HANDLE surface=0;
   uint32_t width,height,pitch;
   uint32_t *buffer;
   struct timeval t1,t2;
   GFXRect r={0,0};
   GFXGetDisplaySize(0,&width,&height);
   GFXCreateSurface(0,&surface,width,height,GPF_ARGB,1);
   r.w=width;r.h=height;
   gettimeofday(&t1,NULL);
   for(int i=0;i<TEST_TIMES;i++){
       GFXFillRect(surface,&r,0xFF000000|(i<<8)|(i+i<<16));
   }
   gettimeofday(&t2,NULL);
   int usedtime=(t2.tv_sec*1000+t2.tv_usec/1000-t1.tv_sec*1000+t1.tv_usec/1000);
   printf("FillSpeed=%fms/frame\r\n",usedtime/(float)TEST_TIMES);

   GFXFlip(surface); 
   GFXDestroySurface(surface);
}

TEST_F(GRAPH,Benchmark_Blit){
   HANDLE mainsurface=0;
   HANDLE surface2;
   uint32_t width,height,pitch;
   struct timeval t1,t2;

   GFXGetDisplaySize(0,&width,&height);
   GFXCreateSurface(0,&mainsurface,width,height,GPF_ARGB,1);//1-->main surface
   GFXCreateSurface(0,&surface2,width,height,GPF_ARGB,0);//soft layer

   GFXFillRect(surface2,NULL,0xFFFF0000);
   gettimeofday(&t1,NULL);
   for(int i=0;i<TEST_TIMES;i++)
      GFXBlit(mainsurface,0,0,surface2,NULL);
   gettimeofday(&t2,NULL);
   int usedtime=(t2.tv_sec*1000+t2.tv_usec/1000-t1.tv_sec*1000+t1.tv_usec/1000);
   printf("BlitSpeed=%fms/frame\r\n",usedtime/(float)TEST_TIMES);

   ASSERT_EQ(0,GFXDestroySurface(mainsurface));
   ASSERT_EQ(0,GFXDestroySurface(surface2));
}
typedef struct {
   int format;
   unsigned int rgb[4];
}TSTPIXEL;

#if 0
TEST_F(GRAPH,Format){
    //this case show four color block ,RED,GREEN,BLUE,WHITE.
    HANDLE surface;
    uint32_t width,height;
    GFXGetDisplaySize(&width,&height);
    TSTPIXEL fpixels[]={
       {GPF_ARGB,    {0xFFFF0000,0xFF00FF00,0xFF0000FF,0xFFFFFFFF}},
       {GPF_ABGR,    {0xFF0000FF,0xFF00FF00,0xFFFF0000,0xFFFFFFFF}},
       {GPF_RGB32,   {0xFF0FF000,0xFF00FF00,0xFF0000FF,0xFFFFFFFF}},
       {GPF_ARGB4444,{0xFF00,0xF0F0,0xF00F,0xFFFF}},
       {GPF_ARGB1555,{0xFC00,0xFFE0,0x801F,0x1FFF}}
    };

    for(int i=0;i<sizeof(fpixels)/sizeof(TSTPIXEL)-3;i++){
        GFXCreateSurface(&surface,width,height,fpixels[i].format,1);
	GFXRect r={100,100,100,100};
	GFXFillRect(surface,&r,0xFFFF0000);
	ASSERT_EQ(getPixel(surface,101,101),fpixels[i].rgb[0]);
	r.x+=110;
	GFXFillRect(surface,&r,0xFF00FF00);
	ASSERT_EQ(getPixel(surface,221,101),fpixels[i].rgb[1]);
	r.x+=110;
	GFXFillRect(surface,&r,0xFF0000FF);
	ASSERT_EQ(getPixel(surface,331,101),fpixels[i].rgb[2]);
	r.x+=110,
	GFXFillRect(surface,&r,0xFFFFFFFF);
	ASSERT_EQ(getPixel(surface,441,101),fpixels[i].rgb[3]);

	GFXFlip(surface);
	GFXDestroySurface(surface);
	GFXSleep(5000);
    }
}
#endif

TEST_F(GRAPH,Blit_Normal){
    HANDLE hwsurface;
    HANDLE swsurface;
    unsigned int width,height;
    GFXGetDisplaySize(0,&width,&height);
    GFXCreateSurface(0,&hwsurface,width,height,GPF_ARGB,1);
    GFXRect r1={0,0,width,height};
    GFXFillRect(hwsurface,&r1,0xFF0000FF);
    GFXFlip(hwsurface);
    sleep(1);

    GFXCreateSurface(0,&swsurface,800,600,GPF_ARGB,0);
    GFXRect r={0,0,800,600};
    GFXFillRect(swsurface,&r,0xFFFF0000);
    GFXBlit(hwsurface,0,0,swsurface,NULL);

    GFXFillRect(swsurface,&r,0xFF00FF00);   
    GFXBlit(hwsurface,100,100,swsurface,NULL);

    GFXFlip(hwsurface);
    sleep(2);  
    GFXDestroySurface(swsurface);
    GFXDestroySurface(hwsurface);
}

TEST_F(GRAPH,Blit_CheckColor){
    HANDLE hwsurface;
    HANDLE swsurface;
    unsigned int width,height;
    GFXRect r;
#define FILLCOLOR 0xFFFF0000
    setRect(r,0,0,200,200);
    GFXGetDisplaySize(0,&width,&height);
    GFXCreateSurface(0,&hwsurface,width,height,GPF_ARGB,1);
    GFXCreateSurface(0,&swsurface,r.w,r.h,GPF_ARGB,0);
    GFXFillRect(hwsurface,NULL,0);
    GFXFillRect(swsurface,NULL,FILLCOLOR);
    
    GFXBlit(hwsurface,r.x,r.y,swsurface,NULL);
    ASSERT_EQ(0xFFFF,CheckColor(hwsurface,&r,FILLCOLOR));

    GFXFillRect(hwsurface,NULL,0);r.x+=100;r.y+=100;
    GFXBlit(hwsurface,r.x,r.y,swsurface,NULL);
    ASSERT_EQ(0xFFFF,CheckColor(hwsurface,&r,FILLCOLOR));

    GFXFillRect(hwsurface,NULL,0);
    setRect(r,0,0,200,200);
    GFXBlit(hwsurface,-100,-100,swsurface,NULL);
    setRect(r,0,0,100,100);
    ASSERT_EQ(0xFFFF,CheckColor(hwsurface,&r,FILLCOLOR));

    GFXFillRect(hwsurface,NULL,0);
    setRect(r,0,0,200,200);
    GFXBlit(hwsurface,width-100,height-100,swsurface,NULL);
    setRect(r,width-100,height-100,100,100);
    ASSERT_EQ(0xFFFF,CheckColor(hwsurface,&r,FILLCOLOR));

    ASSERT_EQ(0,GFXDestroySurface(hwsurface));
    ASSERT_EQ(0,GFXDestroySurface(swsurface));
}

TEST_F(GRAPH,Blit_Range){
    HANDLE hwsurface;
    HANDLE swsurface;
    unsigned int width,height;
    GFXRect r;
    setRect(r,0,0,320,240);
    GFXGetDisplaySize(0,&width,&height);
    GFXCreateSurface(0,&hwsurface,width,height,GPF_ARGB,1);
    GFXCreateSurface(0,&swsurface,r.w,r.h,GPF_ARGB,0);
    GFXFillRect(hwsurface,NULL,0);
     
    for(int x=-10-(int)r.w;x<(int)width+10;x+=10){
        uint32_t color=0xFF000000|(((255-x)&0xFF)<<16)|((x&0xFF)<<8)|((128+x)&0xFF);
        r.x=x;r.y=x;
        GFXFillRect(swsurface,NULL,color);
        GFXBlit(hwsurface,x,x,swsurface,NULL);
        CheckColor(hwsurface,&r,color);
        usleep(50000);
    } 
    ASSERT_EQ(0,GFXDestroySurface(hwsurface));
    ASSERT_EQ(0,GFXDestroySurface(swsurface));
}

#if 0
TEST_F(GRAPH,canvas){
    HANDLE surface;
    uint32_t width,height;
    uint32_t*buffer;
    uint32_t pitch;
    uint64_t tmstart;
    GFXGetDisplaySize(&width,&height);
    GFXCreateSurface(&surface,width,height,0,1);
    GFXLockSurface(surface,(void**)&buffer,&pitch);
    pixman_image_t*img=pixman_image_create_bits(PIXMAN_a8r8g8b8,width,height,buffer,pitch);

    GFXui::FontManager::getInstance().loadFonts("/usr/lib/fonts");    
    GFXui::FontExtents fe;
    GFXui::Canvas c(img);
    c.set_color(0xFFFFFFFF);
    c.fill_rectaGFXe(50,50,1000,650);
    c.rectaGFXe(200,50,300,50);
    c.select_font(64);
    c.get_font_extents(&fe);
    printf("ascent=%f descent=%f height=%f max_advance=%f,%f\r\n",fe.ascent,fe.descent,fe.height,fe.max_x_advance,fe.max_y_advance);
    c.draw_text(200,50,"Hello world!");
    c.set_color(0xFFFF0088);
    c.rectaGFXe(100,200,1000,1);
    c.select_font(40);
    tmstart=gettime();
    c.draw_text(100,200,"The quick brown fox jumps over a lazy dog");
    printf("1used time=%lld\r\n",gettime()-tmstart);
    
    tmstart=gettime();
    c.draw_text(100,200,"The quick brown fox jumps over a lazy dog");
    printf("2used time=%lld\r\n",gettime()-tmstart);
    pixman_image_ref(img);
    GFXUnlockSurface(surface);
    GFXFlip(surface);
    sleep(2);

    GFXDestroySurface(surface);
}
#endif
