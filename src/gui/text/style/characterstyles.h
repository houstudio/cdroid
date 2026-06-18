
#ifndef __CHARACTER_STYLES_H__
#define __CHARACTER_STYLES_H__
#include <text/parcelablespan.h>
namespace cdroid{
class StyleSpan : public CharacterStyle {
public:
    explicit StyleSpan(int style) : mStyle(style) {}
    int getStyle() const { return mStyle; }
private:
    int mStyle;
};

class UnderlineSpan : public CharacterStyle {
};

class StrikethroughSpan : public CharacterStyle {
};
class SubscriptSpan: public CharacterStyle {
};
class SuperscriptSpan: public CharacterStyle {
};
class URLSpan : public CharacterStyle {
public:
    explicit URLSpan(const std::string& url) : mUrl(url) {}
    const std::string& getUrl() const { return mUrl; }
private:
    std::string mUrl;
};

class TypefaceSpan : public CharacterStyle {
public:
    explicit TypefaceSpan(const std::string& family) : mFamily(family) {}
    const std::string& getFamily() const { return mFamily; }
private:
    std::string mFamily;
};

class AbsoluteSizeSpan : public CharacterStyle {
public:
    explicit AbsoluteSizeSpan(int size) : mSize(size) {}
    int getSize() const { return mSize; }
private:
    int mSize;
};

class RelativeSizeSpan : public CharacterStyle {
public:
    explicit RelativeSizeSpan(float proportion) : mProportion(proportion) {}
    float getProportion() const { return mProportion; }
private:
    float mProportion;
};
}/* namespace cdroid */
#endif/*__CHARACTER_STYLES_H__*/
