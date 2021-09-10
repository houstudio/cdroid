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

typedef struct Rect{
    int left;
    int top;
    int width;
    int height;

    int bottom()const{return top+height;}
    int right()const{return left+width;}
    int centerX()const{return left+width/2;}
    int centerY()const{return top+height/2;}
    void set(int x_,int y_,int w,int h);
    void inflate(int dx,int dy);
    bool empty()const;
    void setEmpty();
    void offset(int dx,int dy);
    bool intersect(const Rect&a,const Rect&b);
    bool intersect(const Rect&b);
    bool intersect(int l, int t, int w, int h);
    bool contains(int x,int y)const;
    bool contains(const Rect&a)const;
    bool operator==(const Rect&b)const;
    bool operator!=(const Rect&b)const;
    void Union(const Rect&b);
    void Union(int x,int y,int w,int h);
    static constexpr Rect Make(int x,int y,int w,int h){return Rect{x,y,w,h};}
    static constexpr Rect MakeEmpty(){return Rect{0,0,0,0};};
    static constexpr Rect MakeWH(int w,int h){return Rect{0,0,w,h};}
    static constexpr Rect MakeLTRB(int l,int t,int r,int b){return Rect{l,t,r-l,b-t};}
    static constexpr Rect MakeSize(const SIZE&sz){return Rect{0,0,sz.x,sz.y};}
}RECT;
typedef struct RectFloat{
    float x;
    float y;
    float width;
    float height;
}RectF;
#define MAKERECT(x,y,w,h) RECT::Make(x,y,w,h)

}
#endif

