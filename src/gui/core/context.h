#ifndef __CONTEXT_H__
#define __CONTEXT_H__
#include <string>
#include <iostream>
#include <functional>
#include <cairomm/refptr.h>
#include <cairomm/surface.h>
#include <core/callbackbase.h>
#include <core/attributeset.h>
#include <core/displaymetrics.h>

using namespace Cairo;

namespace cdroid{

class Drawable;
class ColorStateList;
class Context{
public:
    virtual const std::string getTheme()const=0;
    virtual void setTheme(const std::string&theme)=0;
    virtual const DisplayMetrics&getDisplayMetrics()=0;
    virtual int getId(const std::string&)const=0;
    virtual const std::string& getString(const std::string&id,const std::string&lan="")=0;
    static RefPtr<Cairo::ImageSurface> loadImage( std::istream&istream ){
        return Cairo::ImageSurface::create_from_stream(istream);
    }
    virtual std::unique_ptr<std::istream>getInputStream(const std::string&,std::string*outpkg=nullptr)=0;
    virtual RefPtr<Cairo::ImageSurface> getImage(const std::string&resname)=0;
    virtual Drawable* getDrawable(const std::string&resid)=0;
    Drawable* getDrawable(const AttributeSet&atts,const std::string&key){
        return atts.hasAttribute(key)?getDrawable(atts.getString(key)):nullptr;
    }
    virtual int getColor(const std::string&resid)=0;
    virtual int getArray(const std::string&resname,std::vector<std::string>&)=0;
    virtual ColorStateList* getColorStateList(const std::string&resid)=0;
    virtual AttributeSet obtainStyledAttributes(const std::string&resid)=0;
};

}
#endif
