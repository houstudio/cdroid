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
#include <widget/internal_R.h>
#include <widget/editor.h>
#include <widget/editorinfo.h>
#include <text/selection.h>
#include <text/method/textkeylistener.h>
#include <text/method/arrowkeymovementmethod.h>
#include <core/inputmethodmanager.h>
#include <text/textutils.h>
#include <porting/cdlog.h>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET2(EditText,R::attr::editTextStyle)

EditText::EditText(Context*ctx)
    :EditText(ctx,nullptr){
}

EditText::EditText(Context*ctx,const AttributeSet* attrs):EditText(ctx,attrs,cdroid::internal::R::attr::editTextStyle){
}

EditText::EditText(Context*ctx,const AttributeSet* pAttrs,int defStyleAttr)
  :TextView(ctx,pAttrs, defStyleAttr){
    // TextView's ctor evaluates virtual getDefaultEditable() while only the
    // base subobject exists, so C++ dispatches statically to TextView's
    // (false) where Java's super() would reach this override (true). Re-apply
    // AOSP TextView's editable branch (TextView.java "else if (editable)":
    // TextKeyListener + TYPE_CLASS_TEXT) here: setKeyListener installs the
    // listener and derives mInputType from it (= TYPE_CLASS_TEXT). The guard
    // keeps the ctor's branch priority — attrs that already configured the
    // editor (inputType/digits/numeric/phone/autotext) are left untouched.
    // Without this mEditor->mInputType stays TYPE_NULL, onCheckIsTextEditor()
    // is false and the soft keyboard never shows on focus.
    if (getInputType() == EditorInfo::TYPE_NULL) {
        setKeyListener(TextKeyListener::getInstance());
    }
    initEditText();
    // Android-aligned: an EditText's buffer is Editable from construction via
    // setText(EDITABLE) — not a runtime setEditable() conversion. setText also
    // creates the Editor and syncs mTransformed, so the Layout draws the same
    // buffer Editor edits.
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
    // Android EditText.setSelection:Selection::setSelection(&getText(), start, stop).
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
