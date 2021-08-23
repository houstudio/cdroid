#ifndef __CONTEXT_H__
#define __CONTEXT_H__
#include <string>
#include <iostream>
#include <functional>
#include <cairomm/refptr.h>
#include <cairomm/surface.h>
#include <core/callbackbase.h>
using namespace Cairo;

namespace cdroid{

typedef CallbackBase<void> Runnable;
class Drawable;
class ColorStateList;
class Context{
public:
     virtual const std::string& getString(const std::string&id,const std::string&lan="")=0;

     static RefPtr<Cairo::ImageSurface> loadImage( std::istream&istream ){
         return Cairo::ImageSurface::create_from_stream(istream);
     }
     virtual std::unique_ptr<std::istream>getInputStream(const std::string&)=0;
     virtual RefPtr<Cairo::ImageSurface> getImage(const std::string&resname,bool cache=true)=0;
     virtual Drawable* getDrawable(const std::string&resid)=0;
     virtual ColorStateList* getColorStateList(const std::string&resid)=0;
};

}
#endif
