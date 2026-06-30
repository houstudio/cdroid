/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CDROID_KEYLISTENER_H
#define CDROID_KEYLISTENER_H

namespace cdroid{

class View;
class Editable;
class KeyEvent;

class KeyListener {
public:
    virtual ~KeyListener() = default;
    
    /**
     * Return the type of text that this key listener is manipulating,
     * as per InputType.  This is used to determine the mode of the 
     * soft keyboard that is shown for the editor.
     */
    virtual int getInputType() const = 0;
    
    /**
     * If the key listener wants to handle this key, return true,
     * otherwise return false and the caller (i.e. the widget host)
     * will handle the key.
     */
    virtual bool onKeyDown(View* view, Editable& text, int keyCode, const KeyEvent& event) = 0;
    
    /**
     * If the key listener wants to handle this key release, return true,
     * otherwise return false and the caller (i.e. the widget host)
     * will handle the key.
     */
    virtual bool onKeyUp(View* view, Editable& text, int keyCode, const KeyEvent& event) = 0;
    
    /**
     * If the key listener wants to other kinds of key events, return true,
     * otherwise return false and the caller (i.e. the widget host)
     * will handle the key.
     */
    virtual bool onKeyOther(View* view, Editable& text, const KeyEvent& event);
    
    /**
     * Remove the given shift states from the edited text.
     */
    virtual void clearMetaKeyState(View* view, Editable& content, int states) = 0;
};

} // namespace cdroid

#endif // CDROID_KEYLISTENER_H__
