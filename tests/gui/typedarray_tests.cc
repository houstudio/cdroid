// Port of AOSP CTS TypedArrayTest
// (cts/tests/tests/content/src/android/content/res/cts/TypedArrayTest.java,
//  android-12). Adaptations:
//  - testSourceResourceIdFromStyle/Layout deferred: needs the AOSP
//    STYLE_SOURCE_RES_ID column (which style supplied each entry), not tracked.
//  - type18 (font) is declared but not set in Whatever (no fonts in this pak),
//    so EXPECTED_INDEX_COUNT is 18, not CTS's 19.
//  - testRecycle/testAutoCloseable collapse into one case (recycle is a no-op
//    here — the array frees via RAII).
#include <gtest/gtest.h>
#include <core/app.h>
#include <content/resources.h>
#include <content/typedarray.h>
#include <content/typedvalue.h>
#include <core/xmlpullparser.h>
#include "R.h"
#include <widget/internal_R.h>
#include <guienvironment.h>
using namespace cdroid;

namespace {
constexpr int DEFINT = -1;
constexpr float DEFFLOAT = -1.0f;
constexpr int EXPECTED_COLOR = 0xff0000ff;
constexpr int EXPECTED_COLOR_STATE = 0xff00ff00;
constexpr float EXPECTED_DIMENSION = 0.75f;
constexpr int EXPECTED_PIXEL_OFFSET = 10;
constexpr int EXPECTED_PIXEL_SIZE = 18;
constexpr float EXPECTED_FLOAT = 3.14f;
constexpr float EXPECTED_FRACTION = 10.0f;
constexpr int EXPECTED_INT = 365;
constexpr int EXPECTED_INT_ATT = 86400;
constexpr const char* EXPECTED_STRING = "Hello, Android!";
constexpr const char* EXPECTED_TEXT = "TypedArray Test!";
constexpr int EXPECTED_INDEX_COUNT = 18;  // type18 (font) + typeUndefined unset
constexpr int EXPECTED_LENGTH = 20;
constexpr const char* EXPECTED_NON_RESOURCE_STRING = "testNonResourcesString";

// style1 styleable (sentinel-terminated attr id array, C++ analog of int[]).
const uint32_t STYLE1[] = {
    (uint32_t)gui_test::R::attr::type1,  (uint32_t)gui_test::R::attr::type2,
    (uint32_t)gui_test::R::attr::type3,  (uint32_t)gui_test::R::attr::type4,
    (uint32_t)gui_test::R::attr::type5,  (uint32_t)gui_test::R::attr::type6,
    (uint32_t)gui_test::R::attr::type7,  (uint32_t)gui_test::R::attr::type8,
    (uint32_t)gui_test::R::attr::type9,  (uint32_t)gui_test::R::attr::type10,
    (uint32_t)gui_test::R::attr::type11, (uint32_t)gui_test::R::attr::type12,
    (uint32_t)gui_test::R::attr::type13, (uint32_t)gui_test::R::attr::type14,
    (uint32_t)gui_test::R::attr::type15, (uint32_t)gui_test::R::attr::type16,
    (uint32_t)gui_test::R::attr::type17, (uint32_t)gui_test::R::attr::type18,
    (uint32_t)gui_test::R::attr::typeEmpty,
    (uint32_t)gui_test::R::attr::typeUndefined, 0
};
// Attr indices within STYLE1 (order above).
enum {
    kType1 = 0, kType2, kType3, kType4, kType5, kType6, kType7, kType8, kType9,
    kType10, kType11, kType12, kType13, kType14, kType15, kType16, kType17,
    kType18, kTypeEmpty, kTypeUndefined
};

std::unique_ptr<TypedArray> obtainWhatever() {
    return App::getInstance().getTheme().obtainStyledAttributes(
            gui_test::R::style::Whatever, STYLE1);
}
} // namespace

class TYPEDARRAY : public testing::Test {};

// CTS testGetType: the raw type of every entry (unset → TYPE_NULL).
TEST_F(TYPEDARRAY, getType) {
    auto t = obtainWhatever();
    ASSERT_EQ(t->getType(kType1),  (int)TypedValue::TYPE_INT_BOOLEAN);
    ASSERT_EQ(t->getType(kType2),  (int)TypedValue::TYPE_INT_BOOLEAN);
    ASSERT_EQ(t->getType(kType3),  (int)TypedValue::TYPE_INT_COLOR_ARGB8);
    // CDROID deviation (documented): the resolver eagerly flattens style refs
    // (@color/selector → the selector file's TYPE_STRING path); AOSP keeps the
    // raw TYPE_REFERENCE here and resolves per-getter.
    ASSERT_EQ(t->getType(kType4),  (int)TypedValue::TYPE_STRING);
    ASSERT_EQ(t->getType(kType5),  (int)TypedValue::TYPE_DIMENSION);
    ASSERT_EQ(t->getType(kType6),  (int)TypedValue::TYPE_DIMENSION);
    ASSERT_EQ(t->getType(kType7),  (int)TypedValue::TYPE_DIMENSION);
    ASSERT_EQ(t->getType(kType8),  (int)TypedValue::TYPE_STRING);  // flattened ref (see type4)
    ASSERT_EQ(t->getType(kType9),  (int)TypedValue::TYPE_FLOAT);
    ASSERT_EQ(t->getType(kType10), (int)TypedValue::TYPE_FRACTION);
    ASSERT_EQ(t->getType(kType11), (int)TypedValue::TYPE_INT_DEC);
    ASSERT_EQ(t->getType(kType12), (int)TypedValue::TYPE_INT_DEC);
    ASSERT_EQ(t->getType(kType13), (int)TypedValue::TYPE_STRING);
    ASSERT_EQ(t->getType(kType14), (int)TypedValue::TYPE_STRING);
    ASSERT_EQ(t->getType(kType15), (int)TypedValue::TYPE_REFERENCE);
    ASSERT_EQ(t->getType(kType16), (int)TypedValue::TYPE_STRING);
    ASSERT_EQ(t->getType(kType17), (int)TypedValue::TYPE_STRING);
    ASSERT_EQ(t->getType(kTypeEmpty),    (int)TypedValue::TYPE_NULL);
    ASSERT_EQ(t->getType(kTypeUndefined),(int)TypedValue::TYPE_NULL);
}

// CTS testBasics (adapted): length/getResources; position description and
// changing configurations are not tracked by this TypedArray.
TEST_F(TYPEDARRAY, basics) {
    auto t = obtainWhatever();
    ASSERT_EQ(t->length(), (size_t)EXPECTED_LENGTH);
    ASSERT_EQ(&t->getResources(), &App::getInstance().getResources());
}

// CTS testGetAttributes: every typed getter over the styled set.
TEST_F(TYPEDARRAY, getAttributes) {
    auto t = obtainWhatever();

    ASSERT_TRUE(t->getBoolean(kType1, false));
    ASSERT_FALSE(t->getBoolean(kType2, true));

    ASSERT_EQ((uint32_t)EXPECTED_COLOR, t->getColor(kType3, DEFINT));
    auto csl = t->getColorStateList(kType4);
    ASSERT_NE(csl, nullptr);
    ASSERT_EQ(csl->getDefaultColor(), (int)EXPECTED_COLOR_STATE);

    // "0.75px" is unit-independent (px), so no display metric is involved.
    ASSERT_FLOAT_EQ(EXPECTED_DIMENSION, t->getDimension(kType5, DEFFLOAT));

    ASSERT_EQ(EXPECTED_PIXEL_OFFSET, t->getDimensionPixelOffset(kType6, DEFINT));
    ASSERT_EQ(EXPECTED_PIXEL_OFFSET, t->getLayoutDimension(kType6, 0));
    ASSERT_EQ(EXPECTED_PIXEL_SIZE, t->getDimensionPixelSize(kType7, DEFINT));

    ASSERT_NE(t->getDrawable(kType8), nullptr);
    ASSERT_EQ(t->getResourceId(kType8, DEFINT), (uint32_t)gui_test::R::drawable::pass);

    ASSERT_FLOAT_EQ(EXPECTED_FLOAT, t->getFloat(kType9, DEFFLOAT));
    ASSERT_FLOAT_EQ(EXPECTED_FRACTION, t->getFraction(kType10, 10, 10, DEFFLOAT));
    ASSERT_EQ(EXPECTED_INT, t->getInt(kType11, DEFINT));
    ASSERT_EQ(EXPECTED_INT_ATT, t->getInteger(kType12, DEFINT));

    ASSERT_EQ(t->getString(kType13), EXPECTED_STRING);
    // Style-sourced string → NOT a non-resource string; getText returns it.
    ASSERT_EQ(t->getNonResourceString(kType14), "");
    ASSERT_EQ(t->getText(kType14), EXPECTED_TEXT);

    const auto textArray = t->getTextArray(kType15);
    ASSERT_EQ(textArray.size(), (size_t)3);
    ASSERT_EQ(textArray[0], "Easy");
    ASSERT_EQ(textArray[1], "Medium");
    ASSERT_EQ(textArray[2], "Hard");

    // Every set index reads back present (TYPE_NULL entries excluded).
    const size_t indexCount = t->getIndexCount();
    ASSERT_EQ(indexCount, (size_t)EXPECTED_INDEX_COUNT);
    for (size_t i = 0; i < indexCount; i++) {
        const size_t attrIndex = t->getIndex(i);
        ASSERT_TRUE(t->hasValueOrEmpty(attrIndex));
    }
}

// CTS testPeekValue: raw TypedValue out.
TEST_F(TYPEDARRAY, peekValue) {
    auto t = obtainWhatever();
    TypedValue v;
    ASSERT_TRUE(t->peekValue(kType11, &v));
    ASSERT_EQ(v.type, (int)TypedValue::TYPE_INT_DEC);
    ASSERT_EQ(v.data, (uint32_t)EXPECTED_INT);
}

// CTS testHasValue / hasValueOrEmpty with @empty.
TEST_F(TYPEDARRAY, hasValue) {
    auto t = obtainWhatever();
    ASSERT_TRUE(t->hasValue(kType16));
    ASSERT_FALSE(t->hasValue(kTypeEmpty));
    ASSERT_FALSE(t->hasValue(kTypeUndefined));

    ASSERT_TRUE(t->hasValueOrEmpty(kType16));
    ASSERT_TRUE(t->hasValueOrEmpty(kTypeEmpty));
    ASSERT_FALSE(t->hasValueOrEmpty(kTypeUndefined));
}

// CTS testRecycle/testAutoCloseable (adapted): obtain + release via RAII over
// an attr the DEFAULT theme actually carries (CTS uses the TextAppearance set).
TEST_F(TYPEDARRAY, recycle) {
    const uint32_t attrs[] = {(uint32_t)cdroid::internal::R::attr::colorPrimary, 0};
    auto t = App::getInstance().getTheme().obtainStyledAttributes(attrs);
    ASSERT_GT(t->getIndexCount(), (size_t)0);
}

// CTS testNonResourceString (adapted): an AXML-inline string (element string
// pool) IS a non-resource string.
TEST_F(TYPEDARRAY, nonResourceString) {
    auto parser = App::getInstance().getResources().getXml(gui_test::R::xml::test_color);
    const AttributeSet& set = *parser;
    int type;
    while (((type = parser->next()) != XmlPullParser::START_TAG)
            && (type != XmlPullParser::END_DOCUMENT)) {}
    const uint32_t attrs[] = {(uint32_t)gui_test::R::attr::type13, 0};
    auto ta = App::getInstance().getResources().obtainStyledAttributes(&set, attrs);
    ASSERT_NE(ta, nullptr);
    ASSERT_EQ(ta->getIndexCount(), (size_t)1);
    ASSERT_EQ(ta->getNonResourceString(0), EXPECTED_NON_RESOURCE_STRING);
}

// CTS testEmptyXmlAttributeDoesNotFallbackToTheme: an @empty element attr
// wins over the theme; the entry is present but TYPE_NULL.
TEST_F(TYPEDARRAY, emptyXmlAttributeDoesNotFallbackToTheme) {
    auto parser = App::getInstance().getResources().getXml(gui_test::R::xml::empty);
    const AttributeSet& set = *parser;
    int type;
    while (((type = parser->next()) != XmlPullParser::START_TAG)
            && (type != XmlPullParser::END_DOCUMENT)) {}

    auto theme = App::getInstance().getResources().newTheme();
    theme.applyStyle(gui_test::R::style::Whatever, false);

    auto ta = theme.obtainStyledAttributes(&set, STYLE1, 0, 0);
    ASSERT_NE(ta, nullptr);
    ASSERT_TRUE(ta->hasValueOrEmpty(kType1));
    ASSERT_EQ(ta->getType(kType1), (int)TypedValue::TYPE_NULL);
}
