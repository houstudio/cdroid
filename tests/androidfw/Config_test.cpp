// Ported from AOSP frameworks/base/libs/androidfw/tests/Config_test.cpp.
// Adapted: utils/Vector -> std::vector, ASSERT_EQ(a,b) on configs -> compare==0.
#include "resourcetypes.h"
#include <vector>
#include <cstring>
#include "gtest/gtest.h"
namespace cdroid {
static ResTable_config selectBest(const ResTable_config& target, const std::vector<ResTable_config>& configs) {
    ResTable_config best; memset(&best,0,sizeof(best));
    for (const auto& c : configs) { if (!c.match(target)) continue; if (c.isBetterThan(best,&target)) best=c; }
    return best;
}
static ResTable_config buildDensityConfig(int d){ ResTable_config c; memset(&c,0,sizeof(c)); c.density=uint16_t(d); c.sdkVersion=4; return c; }
static bool cfgEq(const ResTable_config& a, const ResTable_config& b){ return a.compare(b)==0; }

TEST(ConfigTest, shouldSelectBestDensity) {
    ResTable_config dev; memset(&dev,0,sizeof(dev)); dev.density=ResTable_config::DENSITY_XHIGH; dev.sdkVersion=21;
    std::vector<ResTable_config> cfgs;
    ResTable_config exp=buildDensityConfig(ResTable_config::DENSITY_HIGH); cfgs.push_back(exp);
    EXPECT_TRUE(cfgEq(exp,selectBest(dev,cfgs)));
    exp=buildDensityConfig(ResTable_config::DENSITY_XXHIGH); cfgs.push_back(exp);
    EXPECT_TRUE(cfgEq(exp,selectBest(dev,cfgs)));
    exp=buildDensityConfig(int(ResTable_config::DENSITY_XXHIGH)-20); cfgs.push_back(exp);
    EXPECT_TRUE(cfgEq(exp,selectBest(dev,cfgs)));
    cfgs.push_back(buildDensityConfig(int(ResTable_config::DENSITY_HIGH)+20));
    EXPECT_TRUE(cfgEq(exp,selectBest(dev,cfgs)));
    exp=buildDensityConfig(ResTable_config::DENSITY_XHIGH); cfgs.push_back(exp);
    EXPECT_TRUE(cfgEq(exp,selectBest(dev,cfgs)));
    exp=buildDensityConfig(ResTable_config::DENSITY_ANY); exp.sdkVersion=21; cfgs.push_back(exp);
    EXPECT_TRUE(cfgEq(exp,selectBest(dev,cfgs)));
}
TEST(ConfigTest, shouldSelectBestDensityWhenNoneSpecified) {
    ResTable_config dev; memset(&dev,0,sizeof(dev)); dev.sdkVersion=21;
    std::vector<ResTable_config> cfgs;
    cfgs.push_back(buildDensityConfig(ResTable_config::DENSITY_HIGH));
    ResTable_config exp=buildDensityConfig(ResTable_config::DENSITY_MEDIUM); cfgs.push_back(exp);
    EXPECT_TRUE(cfgEq(exp,selectBest(dev,cfgs)));
    exp=buildDensityConfig(ResTable_config::DENSITY_ANY); cfgs.push_back(exp);
    EXPECT_TRUE(cfgEq(exp,selectBest(dev,cfgs)));
}
TEST(ConfigTest, shouldMatchRoundQualifier) {
    ResTable_config dev; memset(&dev,0,sizeof(dev));
    ResTable_config r; memset(&r,0,sizeof(r)); r.screenLayout2=ResTable_config::SCREENROUND_YES;
    EXPECT_FALSE(r.match(dev));
    dev.screenLayout2=ResTable_config::SCREENROUND_YES; EXPECT_TRUE(r.match(dev));
    dev.screenLayout2=ResTable_config::SCREENROUND_NO; EXPECT_FALSE(r.match(dev));
    ResTable_config nr; memset(&nr,0,sizeof(nr)); nr.screenLayout2=ResTable_config::SCREENROUND_NO;
    EXPECT_TRUE(nr.match(dev));
}
TEST(ConfigTest, RoundQualifierShouldHaveStableSortOrder) {
    ResTable_config d; memset(&d,0,sizeof(d));
    ResTable_config l=d; l.screenLayout=ResTable_config::SCREENLONG_YES;
    ResTable_config lr=l; lr.screenLayout2=ResTable_config::SCREENROUND_YES;
    ResTable_config lrp=l; lrp.orientation=ResTable_config::ORIENTATION_PORT;
    EXPECT_TRUE(l.compare(lr)<0); EXPECT_TRUE(l.compareLogical(lr)<0);
    EXPECT_TRUE(lr.compare(l)>0); EXPECT_TRUE(lr.compareLogical(l)>0);
    EXPECT_TRUE(lr.compare(lrp)<0); EXPECT_TRUE(lr.compareLogical(lrp)<0);
    EXPECT_TRUE(lrp.compare(lr)>0); EXPECT_TRUE(lrp.compareLogical(lr)>0);
}
TEST(ConfigTest, ScreenShapeHasCorrectDiff) {
    ResTable_config d; memset(&d,0,sizeof(d));
    ResTable_config r=d; r.screenLayout2=ResTable_config::SCREENROUND_YES;
    EXPECT_EQ(d.diff(r), ResTable_config::CONFIG_SCREEN_ROUND);
}
TEST(ConfigTest, RoundIsMoreSpecific) {
    ResTable_config dev; memset(&dev,0,sizeof(dev));
    dev.screenLayout2=ResTable_config::SCREENROUND_YES; dev.screenLayout=ResTable_config::SCREENLONG_YES;
    ResTable_config a; memset(&a,0,sizeof(a));
    ResTable_config b=a; b.screenLayout=ResTable_config::SCREENLONG_YES;
    ResTable_config c=b; c.screenLayout2=ResTable_config::SCREENROUND_YES;
    EXPECT_TRUE(b.isBetterThan(a,&dev)); EXPECT_TRUE(c.isBetterThan(b,&dev));
}
TEST(ConfigTest, ScreenIsWideGamut) {
    ResTable_config d; memset(&d,0,sizeof(d));
    ResTable_config w=d; w.colorMode=ResTable_config::WIDE_COLOR_GAMUT_YES;
    EXPECT_EQ(d.diff(w), ResTable_config::CONFIG_COLOR_MODE);
}
TEST(ConfigTest, ScreenIsHdr) {
    ResTable_config d; memset(&d,0,sizeof(d));
    ResTable_config h=d; h.colorMode=ResTable_config::HDR_YES;
    EXPECT_EQ(d.diff(h), ResTable_config::CONFIG_COLOR_MODE);
}
}  // namespace cdroid
