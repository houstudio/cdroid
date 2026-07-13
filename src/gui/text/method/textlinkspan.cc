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
#include <text/method/textlinkspan.h>

namespace cdroid{

TextLinkSpan::TextLinkSpan(const std::string& url):mUrl(url){
}

void TextLinkSpan::onClick(View& widget)const{
    onClick(widget, INVOCATION_METHOD_UNSPECIFIED);
}

void TextLinkSpan::onClick(View& widget, InvocationMethod invocationMethod)const{
    // AOSP TextLinkSpan.onClick: with smart linkify on (TextClassificationManager),
    // TOUCH asks the TextView for an action mode (requestActionMode) and KEYBOARD/
    // UNSPECIFIED opens the link (handleClick); with smart linkify off it delegates
    // to the TextLink's URLSpan (or handleClick). CDROID has no TextClassifier yet,
    // so we keep the no-smart-linkify path. URL opening itself is still deferred
    // (cf. URLSpan::onClick is a stub) -- TODO wire TextView::handleClick /
    // requestActionMode + TextLink when the textclassifier framework lands.
    (void)widget;
    (void)invocationMethod;
}

}/*endof namespace*/
