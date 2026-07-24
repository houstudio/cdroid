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
   /* Whether this method CONVERTS the typed letters into different text (pinyin:
    * the letters are pinyin and candidates are hanzi, so a candidate tap or space
    * commits a candidate) vs treats them as literal input (english: the typed
    * letters are the text, space is a word separator). The controller uses this
    * only to pick the space-while-composing behaviour. Default false (literal). */
   virtual bool isConversionMethod()const{return false;}

   /* Custom keyboard layout provisioning (per-product customization).
    *
    * Return the layout resource for the given inputType (e.g.
    * "@myprod:xml/keypad.xml", resolved from the product's app pak the same way
    * "@cdroid:xml/qwerty.xml" is, or a filesystem path), or empty to use the
    * IME's built-in default.
    *
    * - A real editor inputType (>= 0): the field-driven keyboard. The value can
    *   inspect class AND variation (e.g. a distinct layout for password). Covers
    *   text / number / phone / datetime.
    * - POPUP (-1): the long-press accent mini-keyboard's container layout. The
    *   popup is not inputType-driven, so it is requested via this sentinel.
    *
    * The 123 symbol page is IME-internal and uses the built-in symbols.xml
    * (not routed through here). Defaults return empty, so the bundled
    * English/Pinyin methods are unchanged -- override in a product subclass to
    * ship custom keyboards without modifying CDROID. */
   static constexpr int POPUP = -1;  // sentinel inputType: request the popup layout
   /* Returns the system-default keyboard set (inherited unchanged by the bundled
    * English/Pinyin methods). Override in a product subclass to customize. */
   virtual std::string getKeyboardLayout(int inputType)const;
};

};
#endif

