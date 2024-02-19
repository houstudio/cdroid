#ifndef __STRING_TOKENIZER_H__
#define __STRING_TOKENIZER_H__
#include <vector>
#include <string>
namespace cdroid{
class StringTokenizer{
private:
    int mCurrentPosition;
    int mNewPosition;
    int mMaxPosition;
    std::string mStr;
    std::string mDelimiters;
    bool mRetDelims;
    bool mDelimsChanged;
    int maxDelimCodePoint;
    bool mHasSurrogates = false;
    std::vector<int> mDelimiterCodePoints;
private:
    void setMaxDelimCodePoint();
    int skipDelimiters(int startPos)const;
    int scanToken(int startPos)const;
    bool isDelimiter(int codePoint)const;
public:
    StringTokenizer(const std::string& str,const std::string& delim, bool returnDelims);
    StringTokenizer(const std::string& str,const std::string& delim);
    StringTokenizer(const std::string& str);
    bool hasMoreTokens();
    std::string nextToken();
    std::string nextToken(const std::string& delim);
    bool hasMoreElements();
    std::string nextElement();
    int countTokens()const;
};
}/*endof namespace*/
#endif/*__STRING_TOKENIZER_H__*/
