#include <core/typeface.h>
namespace cdroid{

Typeface* Typeface::MONOSPACE;
Typeface* Typeface::SANS_SERIF;
Typeface* Typeface::SERIF;
Typeface* Typeface::DEFAULT;

Typeface* Typeface::create(cdroid::Typeface*, int){
    return nullptr;
}

Typeface* Typeface::create(cdroid::Typeface*, int, bool){
    return nullptr;
}

Typeface* Typeface::create(const std::string& familyName,int style){
    return nullptr;
}

Typeface* Typeface::defaultFromStyle(int style){
    return nullptr;
}

}
