#ifndef __COMPLEX_COLOR_H__
#define __COMPLEX_COLOR_H__
namespace cdroid{
class ComplexColor{
public:
    virtual ~ComplexColor()=default;
    virtual bool isStateful()const{ return false; }
    virtual int getDefaultColor()const=0;
    // AOSP: whether this ComplexColor has unresolved theme attributes that
    // applyTheme() could resolve. CDROID has no theme-preload machinery, so the
    // default is false; subclasses with mThemeAttrs (none today) would override.
    virtual bool canApplyTheme()const{ return false; }
};
}/*endof namespace*/
#endif/*__COMPLEX_COLOR_H__*/
