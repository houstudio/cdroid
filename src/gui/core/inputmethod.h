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
public:
   InputMethod();
   virtual ~InputMethod();
   /* Dictionary paths (reserved for a future dictionary-management UI). */
   const std::string& getSysDict()const;
   const std::string& getUserDict()const;
   /* Load system/user dictionaries. Returns true on success. */
   virtual bool loadDicts(const std::string&sys,const std::string&user);
   /* Decode pinyin -> candidates. Returns candidate count (>=0), or <0 if the
    * method does no candidate search (e.g. the English/qwerty method) and the
    * typed char should be committed directly. */
   virtual int search(const std::string&pinyin,std::vector<std::string>&candidates);
   /* Reset the current search/composition workspace. */
   virtual void closeSearch();
   /* Next-word predictions from a committed history string. Returns count (>=0),
    * or <0 if prediction is unsupported. */
   virtual int getPredicts(const std::string&history,std::vector<std::string>&predicts);
   /* Pinyin syllable segmentation of the current input (strings). Returns count
    * (>=0), or <0 if unsupported. */
   virtual int getSpellings(std::vector<std::string>&syllables){return -1;}
   /* The already-fixed composing prefix (chosen so far) for the composing display;
    * empty if unsupported / nothing fixed yet. */
   virtual std::string fixedString()const{return "";}
   /* Two-level / continuous selection (pinyin). choose() fixes candidate[candId]
    * as the current prefix and refills `candidates` with the remaining pinyin's
    * candidate list; returns the new candidate count, or <0 if the method does
    * not support in-composition selection (e.g. the English/qwerty method, which
    * commits each key directly). cancelLastChoice() reverts the last choose and
    * refills `candidates` likewise. */
   virtual int choose(size_t candId,std::vector<std::string>&candidates);
   virtual int cancelLastChoice(std::vector<std::string>&candidates);
   /* Whether choosing a candidate fixes a prefix and continues with more
    * candidates for the unfixed tail (two-level / phrase selection, e.g.
    * pinyin). Default true: any candidate-producing engine is treated as
    * two-level unless it opts out. Single-level engines (word-completion, e.g.
    * the English method) override to false -- a candidate tap commits the whole
    * word instead of fixing a prefix. */
   virtual bool supportsTwoLevel()const{return true;}
};

};
#endif

