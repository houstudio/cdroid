#ifndef __INPUT_METHOD_H__
#define __INPUT_METHOD_H__
#include <string>
#include <vector>
namespace cdroid{

class InputMethod{
protected:
   std::string sysdict;
   std::string userdict;
   std::string keyboardlayout;
public:
   InputMethod(const std::string&layout);
   virtual ~InputMethod();
   const std::string getSysdict()const;
   const std::string getUserDict()const;
   const std::string getKeyboardLayout(int type)const;
   virtual int load_dicts(const std::string&sys,const std::string&user);
   /*return :>=0 success,<0 search is not supportrd,char can be commit directly*/
   virtual int search(const std::string&,std::vector<std::string>&candidates);
   virtual void close_search();
   /*return :>=0 success,<0 predict is not supportrd*/
   virtual int get_predicts(const std::string&,std::vector<std::string>&predicts);
   //Get the segmentation information(the starting positions) of the spelling
   virtual int get_spellings(std::vector<int>&){return 0;}
   virtual int get_spellings(std::vector<std::string>&){return 0;}
};


class GooglePinyin:public InputMethod{
protected:
   void*handle;
public:
   GooglePinyin(const std::string&layout);
   ~GooglePinyin()override;
   int load_dicts(const std::string&sys,const std::string&user)override;
   int search(const std::string&,std::vector<std::string>&candidates)override;
   void close_search()override;
   int get_predicts(const std::string&,std::vector<std::string>&predicts)override;
   int get_spellings(std::vector<int>&)override;
   int get_spellings(std::vector<std::string>&)override;
};

};
#endif

