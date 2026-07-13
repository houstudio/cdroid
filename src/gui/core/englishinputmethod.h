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
#ifndef __ENGLISH_INPUT_METHOD_H__
#define __ENGLISH_INPUT_METHOD_H__
#include <string>
#include <vector>
#include <core/inputmethod.h>

namespace cdroid{

/* Single-level English word-completion method. Holds a word list (a built-in
 * baseline, optionally overridden by a file) and produces prefix completions:
 * search("hel") -> ["hel", "hello", "help", ...]. It does NOT do in-composition
 * selection -- tapping a candidate commits the whole word -- so it opts out of
 * two-level via supportsTwoLevel()==false. The same pattern (a word list +
 * prefix search) can be reused for other Western-script keyboards, each with
 * its own word file and keyboard layout. */
class EnglishInputMethod:public InputMethod{
public:
   EnglishInputMethod();
   /* Load a word-list file (one word per line) to override the built-in list.
    * If the file is missing or empty, the built-in baseline is kept. */
   bool loadDicts(const std::string&sys,const std::string&user)override;
   int search(const std::string&prefix,std::vector<std::string>&candidates)override;
   bool supportsTwoLevel()const override{return false;}
private:
   std::vector<std::string> mWords;
};

}/*endof namespace*/
#endif
