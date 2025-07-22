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
#include <widget/textswitcher.h>
#include <widget/textview.h>

namespace cdroid{

DECLARE_WIDGET(TextSwitcher)

TextSwitcher::TextSwitcher(int w,int h)
    :ViewSwitcher(w,h){
}

TextSwitcher::TextSwitcher(Context*ctx,const AttributeSet&atts)
    :ViewSwitcher(ctx,atts){
}

void TextSwitcher::addView(View* child, int index, ViewGroup::LayoutParams* params){
    if(dynamic_cast<TextView*>(child)==nullptr)
       throw std::runtime_error("TextSwitcher children must be instances of TextView");
    ViewSwitcher::addView(child,index,params);
}

void TextSwitcher::setText(const std::string&text){
    TextView* t = (TextView*) getNextView();
    t->setText(text);
    showNext();
}

void TextSwitcher::setCurrentText(const std::string& text){
    ((TextView*)getCurrentView())->setText(text);
}

std::string TextSwitcher::getAccessibilityClassName()const{
    return "TextSwitcher";
}

}

