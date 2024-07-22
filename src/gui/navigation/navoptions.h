#ifndef __NAV_OPTIONS_H__
#define __NAV_OPTIONS_H__
namespace cdroid{
class Window;
class Intent;
typedef Window Activity;
class NavOptions {
public:
    static constexpr int LAUNCH_SINGLE_TOP = 0x1;
    static constexpr int LAUNCH_DOCUMENT = 0x2;
    static constexpr int LAUNCH_CLEAR_TASK = 0x4;
private:
    int mLaunchMode;
    int mPopUpTo;
    bool mPopUpToInclusive;
    std::string mEnterAnim;
    std::string mExitAnim;
    std::string mPopEnterAnim;
    std::string mPopExitAnim;
public:
    static void addPopAnimationsToIntent(Intent& intent,NavOptions* navOptions);
    static void applyPopAnimationsToPendingTransition(Activity& activity);
    NavOptions(int launchMode,int popUpTo, bool popUpToInclusive,
            const std::string& enterAnim, const std::string& exitAnim,
            const std::string& popEnterAnim, const std::string& popExitAnim);
    bool shouldLaunchSingleTop() const;
    bool shouldLaunchDocument() const;
    bool shouldClearTask() const;
    int  getPopUpTo() const;
    bool isPopUpToInclusive() const;
    const std::string getEnterAnim() const;
    const std::string getExitAnim() const;
    const std::string getPopEnterAnim() const;
    const std::string getPopExitAnim() const;
};
}/*endof namespace*/
#endif/*__NAV_OPTIONS_H__*/
