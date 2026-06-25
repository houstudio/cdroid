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
#include <widget/edittext.h>
#include <widget/editor.h>
#include <core/inputmethodmanager.h>
#include <utils/textutils.h>
#include <porting/cdlog.h>

namespace cdroid{

DECLARE_WIDGET2(EditText,"cdroid:attr/editTextStyle")

EditText::EditText(int w,int h):EditText(std::string(),w,h){
}

EditText::EditText(Context*ctx,const AttributeSet& attrs)
  :TextView(ctx,attrs){
    initEditText();
    mInputType = attrs.getInt("inputType",std::unordered_map<std::string,int>{
		    {"none",TYPE_NONE}    , {"any", TYPE_ANY},
		    {"number",TYPE_NUMBER}, {"text",TYPE_TEXT},
		    {"textPassword",TYPE_PASSWORD},{"ip",TYPE_IP},
		    {"textVisiblePassword",TYPE_PASSWORD}
	  },TYPE_NONE);
    setEditable(true);
}

EditText::EditText(const std::string&txt,int w,int h):TextView(txt,w,h){
    initEditText();
    setFocusable(true);
    setFocusableInTouchMode(true);
    setEditable(true);
}

void EditText::initEditText(){
    mPasswordChar = '*';
    mEditMode = INSERT;
    mInputType = TYPE_NONE;
    afterChanged = nullptr;
    mCaretRect.set(0,0,1,1);
}

void EditText::setTextWatcher(AfterTextChanged ls){
    afterChanged = ls;
}

void EditText::setText(const std::string&txt){
    mBufferType = BufferType::EDITABLE;
    TextView::setText(txt);
    setCaretPos(0);
}

void EditText::setText(CharSequence* text, BufferType type){
    TextView::setText(text, BufferType::EDITABLE);
}

bool EditText::getDefaultEditable() const{
    return true;
}

int EditText::commitText(const std::wstring&ws){
    // Editing logic lives in Editor now (it owns the editable buffer + caret).
    if (getEditor()) return getEditor()->commitText(ws);
    return 0;
}

void EditText::setInputType(INPUTTYPE tp){
    if(tp!=mInputType){
       if(tp==TYPE_PASSWORD||mInputType==TYPE_PASSWORD)
           invalidate(true);
       switch(tp){
       case TYPE_IP:
           setPattern("^((25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))\\.){3}(25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))$");
           break;
       case TYPE_PASSWORD:
           setSingleLine(true);break;
       case TYPE_NUMBER:setPattern("^[1-9]\\d*$");break;
       default:setPattern("");break;
       }mInputType=tp;
    }
}

int EditText::getInputType()const{
    return mInputType;
}

void EditText::setSelection(int start, int stop) {
    if (getEditor()) getEditor()->setSelection(start, stop);
}

void EditText::setSelection(int index) {
    if (getEditor()) getEditor()->setSelection(index);
}

void EditText::selectAll() {
    if (getEditor()) getEditor()->selectAll();
}

void EditText::extendSelection(int index) {
    if (getEditor()) getEditor()->extendSelection(index);
}

void EditText::setEllipsize(TextUtils::TruncateAt ellipsis){
    if (ellipsis == TextUtils::TruncateAt::MARQUEE) {
        FATAL("EditText cannot use the ellipsize mode TextUtils::TruncateAt::MARQUEE");
    }
    TextView::setEllipsize(ellipsis);
}

void EditText::setPattern(const std::string&pattern){
    mInputPattern=TextUtils::utf8tounicode(pattern);
}

void EditText::setEditMode(EDITMODE mode){
    mEditMode=mode;
}

void EditText::setHint(const std::string&txt){
    TextView::setHint(txt);
    if(getText().empty())invalidate(true);
}

void EditText::onFocusChanged(bool focus,int direction,Rect*prevfocusrect){
    InputMethodManager & imm = InputMethodManager::getInstance();
    if(focus){
        imm.setInputType(mInputType);
        imm.focusIn((View*)this);
    }else{
        imm.focusOut((View*)this);
    }
    // Editor handles cursor blink on focus change via TextView::onFocusChanged.
    TextView::onFocusChanged(focus,direction,prevfocusrect);
    if(mInputType!=TYPE_NONE)imm.showIme();
}

bool EditText::onKeyDown(int keyCode,KeyEvent & event){
    if (getEditor()) {
        const bool handled = getEditor()->onKeyDown(keyCode, event);
        if (handled && afterChanged) afterChanged(*this);
        return handled;
    }
    return TextView::onKeyDown(keyCode, event);
}

int EditText::getPasswordChar()const{
    return mPasswordChar;
}

void EditText::setPasswordChar(int ch){
    mPasswordChar = ch;
    if(mInputType==TYPE_PASSWORD)
        invalidate(true);
}

void EditText::onDraw(Canvas&canvas){
    canvas.set_font_size(getTextSize());
    // Caret is drawn by Editor inside TextView::onDraw (when focused/editing).
    TextView::onDraw(canvas);
}

std::string EditText::getAccessibilityClassName()const{
    return "EditText";
}

void EditText::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info){
    TextView::onInitializeAccessibilityNodeInfoInternal(info);
    if (isEnabled()) {
        info.addAction(AccessibilityNodeInfo::ACTION_SET_TEXT);
    }
}
}//end namespace
