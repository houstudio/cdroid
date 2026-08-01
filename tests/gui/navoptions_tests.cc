/*********************************************************************************
 * NavOptions tests — port of androidx NavOptionsBuilderTest (Builder -> getters).
 * NOTE: androidx's popUpTo { saveState = true } maps to the 3-arg setPopUpTo overloads;
 * CDROID's Builder lacks a standalone setPopUpToSaveState (latent gap), so save-state is
 * exercised only via the 3-arg setPopUpTo(route/id, inclusive, saveState).
 *********************************************************************************/
#include <gtest/gtest.h>
#include <navigation/navoptions.h>

using namespace cdroid;

TEST(NavOptions, SingleTop) {
    NavOptions* o = NavOptions::Builder().setLaunchSingleTop(true).build();
    EXPECT_TRUE(o->shouldLaunchSingleTop());
    delete o;
}

TEST(NavOptions, PopUpToRoute) {
    NavOptions* o = NavOptions::Builder().setPopUpTo(std::string("start"), true).build();
    EXPECT_EQ(o->getPopUpToRoute(), "start");
    EXPECT_TRUE(o->isPopUpToInclusive());
    EXPECT_EQ(o->getPopUpToId(), -1);
    delete o;
}

TEST(NavOptions, PopUpToId) {
    NavOptions* o = NavOptions::Builder().setPopUpTo(42, false).build();
    EXPECT_EQ(o->getPopUpToId(), 42);
    EXPECT_FALSE(o->isPopUpToInclusive());
    EXPECT_TRUE(o->getPopUpToRoute().empty());
    delete o;
}

TEST(NavOptions, Animations) {
    NavOptions* o = NavOptions::Builder()
        .setEnterAnim("enter").setExitAnim("exit")
        .setPopEnterAnim("pe").setPopExitAnim("px").build();
    EXPECT_EQ(o->getEnterAnim(), "enter");
    EXPECT_EQ(o->getExitAnim(), "exit");
    EXPECT_EQ(o->getPopEnterAnim(), "pe");
    EXPECT_EQ(o->getPopExitAnim(), "px");
    delete o;
}

TEST(NavOptions, RestoreState) {
    NavOptions* o = NavOptions::Builder().setRestoreState(true).build();
    EXPECT_TRUE(o->shouldRestoreState());
    delete o;
}

TEST(NavOptions, PopUpToRouteSaveState) {
    NavOptions* o = NavOptions::Builder().setPopUpTo(std::string("start"), true, true).build();
    EXPECT_EQ(o->getPopUpToRoute(), "start");
    EXPECT_TRUE(o->isPopUpToInclusive());
    EXPECT_TRUE(o->shouldPopUpToSaveState());
    delete o;
}

TEST(NavOptions, PopUpToIdSaveState) {
    NavOptions* o = NavOptions::Builder().setPopUpTo(7, false, true).build();
    EXPECT_EQ(o->getPopUpToId(), 7);
    EXPECT_TRUE(o->shouldPopUpToSaveState());
    delete o;
}
