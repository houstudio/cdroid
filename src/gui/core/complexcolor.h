#ifndef __COMPLEX_COLOR_H__
#define __COMPLEX_COLOR_H__
namespace cdroid{
class ComplexColor{
public:
   virtual bool isStateful()const{ return false; }
   virtual int getDefaultColor()const=0;
};
}/*endof namespace*/
#endif/*__COMPLEX_COLOR_H__*/
