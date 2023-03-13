#ifndef __INTENT_H__
#define __INTENT_H__
#include <string>
namespace cdroid{

class Intent{
private:
   std::string mAction;
public:
   Intent(const std::string&);
   bool operator==(const Intent&other)const;
};

}/*endof namespace*/
#endif
