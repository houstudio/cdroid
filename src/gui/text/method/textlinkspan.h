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
#ifndef __TEXT_LINK_SPAN_H__
#define __TEXT_LINK_SPAN_H__
#include <string>
#include <text/parcelablespan.h>   // CharacterStyle (ClickableSpan base)
#include <text/textpaint.h>        // TextPaint (ClickableSpan::updateDrawState)
#include <text/style/clickablespan.h>
namespace cdroid{
class View;

/* AOSP android.view.textclassifier.TextLinks.TextLinkSpan.
 *
 * A ClickableSpan backing a smart-linkify'd region of text. In AOSP it carries
 * a TextLink (entity type + URLSpan) and onClick branches on
 * TextClassificationManager's smart-linkify setting: touch -> request an action
 * mode over the link, keyboard/unspecified -> open it. CDROID has no
 * TextClassifier framework yet, so this port keeps the span shape + the URL
 * payload; onClick takes the no-smart-linkify path (open the URL), and the
 * action-mode / TextLink paths are deferred. */
class TextLinkSpan:public ClickableSpan{
public:
    /* How the click span is triggered (AOSP hidden @IntDef). */
    enum InvocationMethod{
        INVOCATION_METHOD_UNSPECIFIED = -1,
        INVOCATION_METHOD_TOUCH      = 0,
        INVOCATION_METHOD_KEYBOARD   = 1,
    };
    explicit TextLinkSpan(const std::string& url);
    void onClick(View& widget)const override;
    void onClick(View& widget,InvocationMethod invocationMethod)const;
    const std::string& getUrl()const{ return mUrl; }
    TextLinkSpan* clone()const override{ return new TextLinkSpan(*this); }
private:
    std::string mUrl;
};
}/*endof namespace*/
#endif/*__TEXT_LINK_SPAN_H__*/
