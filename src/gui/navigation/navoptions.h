#ifndef __NAV_OPTIONS_H__
#define __NAV_OPTIONS_H__
#include <core/bundle.h>
namespace cdroid{
class Window;
class Intent;
typedef Window Activity;
class NavOptions {
private:
    static constexpr const char*const KEY_LAUNCH_MODE = "launchMode";
    static constexpr const char*const KEY_POP_UP_TO = "popUpTo";
    static constexpr const char*const KEY_POP_UP_TO_INCLUSIVE = "popUpToInclusive";
    static constexpr const char*const KEY_ENTER_ANIM = "enterAnim";
    static constexpr const char*const KEY_EXIT_ANIM = "exitAnim";
    static constexpr const char*const KEY_POP_ENTER_ANIM = "popEnterAnim";
    static constexpr const char*const KEY_POP_EXIT_ANIM = "popExitAnim";
public:
    static constexpr int LAUNCH_SINGLE_TOP = 0x1;
    static constexpr int LAUNCH_DOCUMENT = 0x2;
    static constexpr int LAUNCH_CLEAR_TASK = 0x4;
private:
    int mLaunchMode;
    bool mPopUpToInclusive;
    std::string mPopUpTo;
    std::string mEnterAnim;
    std::string mExitAnim;
    std::string mPopEnterAnim;
    std::string mPopExitAnim;
private:
    Bundle* toBundle();
    static NavOptions* fromBundle(const Bundle& b);
public:
    class Builder;
    static void addPopAnimationsToIntent(Intent& intent,NavOptions* navOptions);
    static void applyPopAnimationsToPendingTransition(Activity& activity);
    NavOptions(int launchMode,const std::string& popUpTo, bool popUpToInclusive, const std::string& enterAnim,
         const std::string& exitAnim, const std::string& popEnterAnim, const std::string& popExitAnim);
    bool shouldLaunchSingleTop() const;
    bool shouldLaunchDocument() const;
    bool shouldClearTask() const;
    const std::string  getPopUpTo() const;
    bool isPopUpToInclusive() const;
    const std::string getEnterAnim() const;
    const std::string getExitAnim() const;
    const std::string getPopEnterAnim() const;
    const std::string getPopExitAnim() const;
};

class NavOptions::Builder {
    int mLaunchMode;
    std::string mPopUpTo;
    bool mPopUpToInclusive;
    std::string mEnterAnim;
    std::string mExitAnim;
    std::string mPopEnterAnim;
    std::string mPopExitAnim;
public:
    Builder();

    Builder& setLaunchSingleTop(bool singleTop);
    Builder& setLaunchDocument(bool launchDocument);

    Builder& setClearTask(bool clearTask);
    Builder& setPopUpTo(const std::string& destinationId, bool inclusive);
    Builder& setEnterAnim(const std::string& enterAnim);
    Builder& setExitAnim(const std::string& exitAnim);
    Builder& setPopEnterAnim(const std::string& popEnterAnim);
    Builder& setPopExitAnim(const std::string& popExitAnim);
    NavOptions* build();
};

}/*endof namespace*/
#endif/*__NAV_OPTIONS_H__*/
