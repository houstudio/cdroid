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
#ifndef __NAVDEEPLINKREQUEST_H__
#define __NAVDEEPLINKREQUEST_H__
/*********************************************************************************
 * Port of androidx.navigation.NavDeepLinkRequest. A (uri, action, mimeType)
 * triple used to match against NavDeepLink patterns and to navigate.
 *********************************************************************************/
#include <string>
namespace cdroid{

class NavDeepLinkRequest{
public:
    class Builder;
    NavDeepLinkRequest(const std::string& uri, const std::string& action, const std::string& mimeType)
        : mUri(uri), mAction(action), mMimeType(mimeType){}
    const std::string& getUri() const { return mUri; }
    const std::string& getAction() const { return mAction; }
    const std::string& getMimeType() const { return mMimeType; }
private:
    std::string mUri;
    std::string mAction;
    std::string mMimeType;
};

class NavDeepLinkRequest::Builder{
public:
    Builder& setUri(const std::string& uri){ mUri = uri; return *this; }
    Builder& setAction(const std::string& action){ mAction = action; return *this; }
    Builder& setMimeType(const std::string& mimeType){ mMimeType = mimeType; return *this; }
    NavDeepLinkRequest* build(){ return new NavDeepLinkRequest(mUri, mAction, mMimeType); }
private:
    std::string mUri;
    std::string mAction;
    std::string mMimeType;
};

}//namespace cdroid
#endif
