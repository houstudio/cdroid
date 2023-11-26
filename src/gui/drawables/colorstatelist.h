#ifndef __COLOR_STATE_LIST_H__
#define __COLOR_STATE_LIST_H__
#include <vector>
#include <iostream>
#include <core/context.h>
#include <core/attributeset.h>

namespace cdroid{
class ComplexColor{
public:
   virtual bool isStateful()const{ return false; }
   virtual int getDefaultColor()const=0;
};

class ColorStateList:public ComplexColor{
private:
    static std::vector<std::vector<int>>EMPTY;
    int mDefaultColor;
    bool mIsOpaque;
    int mChangingConfigurations;
    std::vector<int>mColors;
    std::vector<std::vector<int>>mStateSpecs;
    void onColorsChanged();
    int modulateColorAlpha(int baseColor, float alphaMod)const;
public:
    ColorStateList();
    ColorStateList(const ColorStateList&other);
    ColorStateList(const std::vector<std::vector<int>>&states,const std::vector<int>&colors);
    int addStateColor(const std::vector<int>&stateSet,int color);
    int getDefaultColor()const override;
    bool isOpaque()const;
    bool isStateful()const override;
    bool hasFocusStateSpecified()const;
    ColorStateList*withAlpha(int alpha)const;
    ColorStateList&operator=(const ColorStateList&other);
    bool operator!=(const ColorStateList&other)const;
    bool operator==(const ColorStateList&other)const;
    int getChangingConfigurations()const;
    int getColorForState(const std::vector<int>&stateSet, int defaultColor)const;
    const std::vector<std::vector<int>>& getStates()const;
    const std::vector<int>& getColors()const;
    bool hasState(int state)const ;
    void dump()const;
    static ColorStateList*valueOf(int color);
    static ColorStateList*fromStream(Context*ctx,std::istream&is,const std::string&resname,
	          const std::string&package=std::string());
    static ColorStateList*inflate(Context*ctx,const std::string&resname);
};

}
#endif
