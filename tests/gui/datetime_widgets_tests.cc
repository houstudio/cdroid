#include <gtest/gtest.h>
#include <cdroid.h>
#include <widget/timepicker.h>
#include <widget/datepicker.h>
#include <widget/calendarview.h>
#include <view/layoutinflater.h>
#include <content/dateformatsymbols.h>
#include "R.h"

// Date/time widget regressions: locale symbol resolution, construction/
// destruction (a dtor double-free once lived in RadialTimePickerView and
// SimpleMonthView), and full-page inflation (a ctor that throws would make
// the picker silently disappear from the inflated tree).
class DATETIME_WIDGETS:public testing::Test{
   public:
    void SetUp(){}
    void TearDown(){}
};

TEST_F(DATETIME_WIDGETS, zh_symbol_resolution){
    // The zh tag family resolves real CLDR data regardless of the runtime env.
    for (const char* tag : {"zh", "zh-CN", "zh-Hans", "zh-Hans-CN"}) {
        const cdroid::Locale loc = cdroid::Locale::forLanguageTag(tag);
        const cdroid::DateFormatSymbols dfs(loc);
        const auto& amPm = dfs.getAmPmStrings();
        ASSERT_EQ(2u, amPm.size()) << tag;
        EXPECT_EQ(u8"上午", amPm[0]) << tag;
        EXPECT_EQ(u8"下午", amPm[1]) << tag;
        ASSERT_GT(dfs.getWeekdays().size(), 1u) << tag;
        EXPECT_EQ(u8"星期日", dfs.getWeekdays()[1]) << tag;
    }
    // Underscore ids are not BCP-47; the parser keeps them out and the
    // English fallback applies (graceful degradation, documented).
    const cdroid::DateFormatSymbols dfsUnderscore(
            cdroid::Locale::forLanguageTag("zh_CN"));
    EXPECT_EQ("AM", dfsUnderscore.getAmPmStrings()[0]);
    EXPECT_EQ("Sunday", dfsUnderscore.getWeekdays()[1]);
}

TEST_F(DATETIME_WIDGETS, configuration_locale_seed_is_stable){
    // An untouched Configuration is lazily seeded from LocaleList::getDefault()
    // (env-derived). The seed must be sticky: it once alternated und/<locale>
    // across getLocales() calls because the deprecated `locale` field was not
    // written along with the list, so the AOSP reconcile cleared it again.
    cdroid::Context* ctx = &cdroid::App::getInstance();
    const std::string first = ctx->getResources().getConfiguration()
            .getLocales().get(0).toLanguageTag();
    ASSERT_FALSE(first.empty());
    ASSERT_NE("und", first);
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(first, ctx->getResources().getConfiguration()
                .getLocales().get(0).toLanguageTag()) << "read #" << i;
    }
}

TEST_F(DATETIME_WIDGETS, construct_and_destroy){
    // Stack construct + destruct each picker host; dtors must not double-free
    // (the accessibility delegate is owned by ~View, not the subclasses).
    cdroid::Context* ctx = &cdroid::App::getInstance();
    const char* names[] = {"TimePicker", "DatePicker", "CalendarView"};
    try { cdroid::TimePicker v(ctx);   SUCCEED(); } catch (const std::exception& e) { ADD_FAILURE() << names[0] << " threw: " << e.what(); }
    try { cdroid::DatePicker v(ctx);   SUCCEED(); } catch (const std::exception& e) { ADD_FAILURE() << names[1] << " threw: " << e.what(); }
    try { cdroid::CalendarView v(ctx); SUCCEED(); } catch (const std::exception& e) { ADD_FAILURE() << names[2] << " threw: " << e.what(); }
}

TEST_F(DATETIME_WIDGETS, inflate_datetime_page){
    cdroid::Context* ctx = &cdroid::App::getInstance();
    try {
        cdroid::View* page = cdroid::LayoutInflater::from(ctx)->inflate(
                gui_test::R::layout::dt_probe, nullptr, false);
        ASSERT_NE(nullptr, page);
        // A ctor that throws makes that picker silently disappear from the
        // tree; a healthy page carries all 5 pickers.
        EXPECT_EQ(5, ((cdroid::ViewGroup*)page)->getChildCount())
            << "some pickers failed to inflate";
        delete page;
    } catch (const std::exception& e) {
        ADD_FAILURE() << "inflate threw: " << e.what();
    }
}
