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
#include <text/selection.h>
#include <text/inputtype.h>
#include <text/method/arrowkeymovementmethod.h>
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
    setInputType(attrs.getInt("inputType",std::unordered_map<std::string,int>{
		    {"none",  (int)InputType::TYPE_NULL},
		    {"any",   (int)InputType::TYPE_CLASS_TEXT},
		    {"text",  (int)InputType::TYPE_CLASS_TEXT},
		    {"number",(int)InputType::TYPE_CLASS_NUMBER},
		    {"textPassword",        InputType::TYPE_CLASS_TEXT | (int)InputType::TYPE_TEXT_VARIATION_PASSWORD},
		    {"textVisiblePassword", InputType::TYPE_CLASS_TEXT | (int)InputType::TYPE_TEXT_VARIATION_PASSWORD},
		    {"ip",   (int)InputType::TYPE_CLASS_TEXT}
	  }, (int)InputType::TYPE_NULL));
    // Android-aligned: an EditText's buffer is Editable from construction via
    // setText(EDITABLE) — not a runtime setEditable() conversion. setText also
    // creates the Editor and syncs mTransformed, so the Layout draws the same
    // buffer Editor edits.
    setText(mText, BufferType::EDITABLE);
}

EditText::EditText(const std::string&txt,int w,int h):TextView(txt,w,h){
    initEditText();
    setFocusable(true);
    setFocusableInTouchMode(true);
    // Android-aligned: build the Editable buffer via setText(EDITABLE) instead of
    // a setEditable() conversion (not an Android API). setText creates the Editor
    // and keeps mText/mTransformed/Layout on the same live SpannableStringBuilder.
    setText(mText, BufferType::EDITABLE);
}

void EditText::initEditText(){
    afterChanged = nullptr;
    // Android's default movement method for editable text — arrow/page/home/end
    // navigation + shift-select via the standard android.text.method path.
    setMovementMethod(ArrowKeyMovementMethod::getInstance());
}

void EditText::setTextWatcher(AfterTextChanged ls){
    afterChanged = ls;
}

void EditText::setText(const std::string&txt){
    mBufferType = BufferType::EDITABLE;
    TextView::setText(txt);
}

void EditText::setText(CharSequence* text, BufferType type){
    TextView::setText(text, BufferType::EDITABLE);
}

Editable& EditText::getText() {
    CharSequence& text = TextView::getText();
    // This can only happen during construction.
    //if (text == nullptr) { return nullptr; }
    if (dynamic_cast<Editable*>(&text)) {
        return dynamic_cast<Editable&>(text);
    }
    TextView::setText(&text, BufferType::EDITABLE);
    return dynamic_cast<Editable&>(TextView::getText());
}

bool EditText::getDefaultEditable() const{
    return true;
}

int EditText::commitText(const std::wstring&ws){
    // Editing logic lives in Editor now (it owns the editable buffer + caret).
    if (getEditor()) return getEditor()->commitText(ws);
    return 0;
}

void EditText::setSelection(int start, int stop) {
    // Android EditText.setSelection: Selection.setSelection(getText(), start, stop).
    if (Spannable* e = getEditableText()) Selection::setSelection(e, start, stop);
}

void EditText::setSelection(int index) {
    // Android EditText.setSelection(index): Selection.setSelection(getText(), index).
    if (Spannable* e = getEditableText()) Selection::setSelection(e, index);
}

void EditText::selectAll() {
    // Android EditText.selectAll: Selection::selectAll(&getText());
    if (Spannable* e = getEditableText()) Selection::selectAll(e);
}

void EditText::extendSelection(int index) {
    // Android EditText.extendSelection: Selection.extendSelection(getText(), index).
    if (Spannable* e = getEditableText()) Selection::extendSelection(e, index);
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


void EditText::onFocusChanged(bool focus,int direction,Rect*prevfocusrect){
    InputMethodManager & imm = InputMethodManager::getInstance();
    if(focus){
        imm.setInputType(getInputType());
        imm.focusIn((View*)this);
    }else{
        imm.focusOut((View*)this);
    }
    // Editor handles cursor blink on focus change via TextView::onFocusChanged.
    TextView::onFocusChanged(focus,direction,prevfocusrect);
    if(getInputType()!=InputType::TYPE_NULL)imm.showIme();
}

bool EditText::onKeyDown(int keyCode,KeyEvent & event){
    if (getEditor()) {
        const bool handled = getEditor()->onKeyDown(keyCode, event);
        if (handled && afterChanged) afterChanged(*this);
        return handled;
    }
    return TextView::onKeyDown(keyCode, event);
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
