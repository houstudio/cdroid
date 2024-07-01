#include <navigation/navdeeplink.h>
namespace cdroid{
#if 1
//static char* SCHEME_PATTERN = "^(\\w+-)*\\w+:";
NavDeepLink::NavDeepLink(const std::string& uri) {
    /*StringBuffer uriRegex = new StringBuffer("^");

    if (!SCHEME_PATTERN.matcher(uri).find()) {
        uriRegex.append("http[s]?://");
    }
    Pattern fillInPattern = Pattern.compile("\\{(.+?)\\}");
    Matcher matcher = fillInPattern.matcher(uri);
    while (matcher.find()) {
        String argName = matcher.group(1);
        mArguments.add(argName);
        matcher.appendReplacement(uriRegex, "");
        uriRegex.append("(.+?)");
    }
    matcher.appendTail(uriRegex);
    mPattern = Pattern.compile(uriRegex.toString());*/
}

bool NavDeepLink::matches(/*@NonNull Uri*/const std::string& deepLink)const {
    return false;//mPattern.matcher(deepLink.toString()).matches();
}

Bundle NavDeepLink::getMatchingArguments(/*@NonNull Uri*/const std::string& deepLink) {
#if 0
    Matcher matcher = mPattern.matcher(deepLink.toString());
    if (!matcher.matches()) {
        return nullptr;
    }
    Bundle bundle = new Bundle();
    int size = mArguments.size();
    for (int index = 0; index < size; index++) {
        String argument = mArguments.get(index);
        bundle.putString(argument, Uri.decode(matcher.group(index + 1)));
    }
#endif
    return Bundle();
}
#endif
}/*endof namespace*/
