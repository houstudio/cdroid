/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
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

