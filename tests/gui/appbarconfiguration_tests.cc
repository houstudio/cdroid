/*********************************************************************************
 * AppBarConfiguration tests — port of androidx.navigation navigation-ui
 * AppBarConfigurationTest (the subset CDROID implements; CDROID's configuration
 * is route-based where androidx is id-based, so the id -> route tests map 1:1).
 *
 * Ported: top-level routes from varargs-style adds and from a set,
 * setOpenableLayout, setFallbackOnNavigateUpListener, isTopLevelDestination
 * for member/non-member routes.
 *
 * Skipped: testTopLevelFromGraph and the topLevelMenu Toolbar-based tests
 * (no graph/menu Builder ctor in CDROID — top levels are set as routes).
 *********************************************************************************/
#include <gtest/gtest.h>
#include <navigation/appbarconfiguration.h>
#include <widget/openable.h>

using namespace cdroid;

namespace {
// Minimal Openable stand-in (androidx uses a real DrawerLayout; only identity matters here).
class FakeDrawerLayout : public Openable{
public:
    bool isOpen() override { return false; }
    void open() override {}
    void close() override {}
};
} // namespace

TEST(AppBarConfiguration, TopLevelFromVarargs) {
    AppBarConfiguration::Builder builder;
    builder.addTopLevelRoute("1");
    AppBarConfiguration* config = builder.build();
    EXPECT_EQ(config->getTopLevelDestinationRoutes(), std::set<std::string>({"1"}));
    delete config;
}

TEST(AppBarConfiguration, TopLevelFromSet) {
    AppBarConfiguration::Builder builder;
    builder.setTopLevelRoutes({"1", "2"});
    AppBarConfiguration* config = builder.build();
    EXPECT_EQ(config->getTopLevelDestinationRoutes(), std::set<std::string>({"1", "2"}));
    delete config;
}

TEST(AppBarConfiguration, SetOpenableLayout) {
    AppBarConfiguration::Builder builder;
    FakeDrawerLayout drawerLayout;
    builder.setOpenableLayout(&drawerLayout);
    AppBarConfiguration* config = builder.build();
    EXPECT_EQ(config->getOpenableLayout(), &drawerLayout);
    delete config;
}

TEST(AppBarConfiguration, SetFallbackOnNavigateUpListener) {
    AppBarConfiguration::Builder builder;
    bool called = false;
    AppBarConfiguration::OnNavigateUpListener listener = [&called]{ called = true; return false; };
    builder.setFallbackOnNavigateUpListener(listener);
    AppBarConfiguration* config = builder.build();
    const auto& stored = config->getFallbackOnNavigateUpListener();
    ASSERT_TRUE((bool)stored);
    EXPECT_FALSE(stored());
    EXPECT_TRUE(called);
    delete config;
}

TEST(AppBarConfiguration, IsTopLevelDestination) {
    AppBarConfiguration::Builder builder;
    builder.setTopLevelRoutes({"1", "2"});
    AppBarConfiguration* config = builder.build();
    EXPECT_TRUE (config->isTopLevelDestination("1"));
    EXPECT_TRUE (config->isTopLevelDestination("2"));
    EXPECT_FALSE(config->isTopLevelDestination("3"));
    delete config;
}
