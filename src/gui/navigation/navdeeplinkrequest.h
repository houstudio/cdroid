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
