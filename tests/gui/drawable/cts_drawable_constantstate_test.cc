// AOSP CTS drawable test port (Drawable_ConstantStateTest.java). CDROID's ConstantState
// (drawable.h:75) only declares the no-arg newDrawable() and getChangingConfigurations(); the
// Android overloads newDrawable(Resources) / newDrawable(Resources, Theme) and canApplyTheme()
// are NOT ported (CDROID has no Resources/Theme model on ConstantState). testNewDrawable is
// therefore trimmed to the no-arg path, and testCanApplyTheme is skipped (no such method).
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/Drawable_ConstantStateTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/drawable.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// CTS MockConstantState: returns null from newDrawable() and 0 from getChangingConfigurations().
// Both methods are pure-virtual in CDROID's ConstantState, so a concrete subclass is required.
class MockConstantState : public Drawable::ConstantState {
public:
    Drawable* newDrawable() override { return nullptr; }
    int getChangingConfigurations() const override { return 0; }
};
} // namespace

class CtsDrawableConstantStateTest : public testing::Test {};

TEST_F(CtsDrawableConstantStateTest, testNewDrawable) {
    MockConstantState cs;
    // CDROID only exposes the no-arg newDrawable(); the Resources/Theme overloads from CTS do
    // not exist on ConstantState (CDROID has no Resources/Theme), so they are not ported here.
    EXPECT_EQ(nullptr, cs.newDrawable());
}

// CTS testCanApplyTheme asserts ConstantState.canApplyTheme()==false. CDROID's ConstantState
// has no canApplyTheme() method (theming is not modeled), so this case is not ported.
