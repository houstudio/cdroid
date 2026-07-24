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
#ifndef __CDROID_EDITVIEW_H__
#define __CDROID_EDITVIEW_H__
#include <widget/textview.h>

namespace cdroid{

class EditText:public TextView{
private:
    void initEditText();
public:
    DECLARE_UIEVENT(void,AfterTextChanged,EditText&);
protected:
    AfterTextChanged afterChanged;
    bool getDefaultEditable()const override;
    int commitText(const std::wstring&ws)override;
public:
    EditText(int w,int h);
    EditText(const std::string&txt,int w,int h);
    EditText(Context*ctx,const AttributeSet&attrs);
    void setText(const std::string&txt)override;
    void setPattern(const std::string&pattern);
    void setSelection(int start, int stop);
    void setSelection(int index);
    void selectAll();
    void extendSelection(int index);
    void setEllipsize(TextUtils::TruncateAt ellipsis)override;
    void setText(CharSequence* text, BufferType type)override;
    virtual void setTextWatcher(AfterTextChanged ls);
    Editable& getText() override;
    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
};

}//endof cdroid

#endif
