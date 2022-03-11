#pragma once
#include <core/rect.h>
namespace cdroid{

class Insets{
public:
    int left;
    int top;
    int right;
    int bottom;
    const static Insets NONE;
private:
    Insets(int left, int top, int right, int bottom);
public:
    Insets();
    void set(int l,int t,int r,int b);
    static Insets of(int left, int top, int right, int bottom);
    static Insets of(const Rect& r);
    bool operator==(const Insets&o)const;
    bool operator!=(const Insets&o)const;
};

}
