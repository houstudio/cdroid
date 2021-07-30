#ifndef __UIBASE_H__
#define __UIBASE_H__
#define    DISALLOW_COPY_AND_ASSIGN(TypeName) \
    private: TypeName(const TypeName&);                \
    private: TypeName& operator=(const TypeName&)
namespace cdroid{

typedef struct Point{
    int x;
    int y;
    void set(int x,int y);
    static constexpr Point Make(int x,int y){return {x,y};}
}POINT;
#define MAKEPOINT(x,y) POINT::Make(x,y)

typedef struct Size{
    int x;
    int y;
    void set(int x_,int y_){x=x_;y=y_;}
    int width(){return x;}
    int height(){return y;}
    static constexpr Size Make(int x,int y){return {x,y};}
    static constexpr Size MakeEmpty(){return {0,0};}
}SIZE;
#define MAKESIZE(x,y) SIZE::Make(x,y)

typedef struct Rectangle{
    int x;
    int y;
    int width;
    int height;

    int bottom()const{return y+height;}
    int right()const{return x+width;}
    int centerX()const{return x+width/2;}
    int centerY()const{return y+height/2;}
    void set(int x_,int y_,int w,int h);
    void inflate(int dx,int dy);
    bool empty()const;
    void setEmpty();
    void offset(int dx,int dy);
    bool intersect(const Rectangle&a,const Rectangle&b);
    bool intersect(const Rectangle&b);
    bool intersect(int l, int t, int r, int b);
    bool contains(int x,int y)const;
    bool contains(const Rectangle&a)const;
    bool operator==(const Rectangle&b)const;
    bool operator!=(const Rectangle&b)const;
    static constexpr Rectangle Make(int x,int y,int w,int h){return Rectangle{x,y,w,h};}
    static constexpr Rectangle MakeEmpty(){return Rectangle{0,0,0,0};};
    static constexpr Rectangle MakeWH(int w,int h){return Rectangle{0,0,w,h};}
    static constexpr Rectangle MakeLTRB(int l,int t,int r,int b){return Rectangle{l,t,r-l,b-t};}
    static constexpr Rectangle MakeSize(const SIZE&sz){return Rectangle{0,0,sz.x,sz.y};}
}Rect,RECT;
typedef struct RectFloat{
    float x;
    float y;
    float width;
    float height;
}RectF;
#define MAKERECT(x,y,w,h) RECT::Make(x,y,w,h)

}
#endif

