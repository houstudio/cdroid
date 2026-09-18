// Ported from AOSP frameworks/base/libs/androidfw/tests/DynamicRefTable_test.cpp.
// Validates our DynamicRefTable port against AOSP's own expectations.
#include "resourcetypes.h"
#include "gtest/gtest.h"
namespace cdroid {
TEST(DynamicRefTableTest, LookupSharedLibSelfReferences) {
  // Shared library
  DynamicRefTable shared_table(0x02, /* appAsLib */ false);
  shared_table.addMapping(0x00, 0x02);
  Res_value value;
  value.dataType = Res_value::TYPE_REFERENCE;
  value.data = 0x00010000;
  ASSERT_EQ(shared_table.lookupResourceValue(&value), NO_ERROR);
  EXPECT_EQ(value.data, 0x02010000);

  // App loaded as a shared library
  DynamicRefTable shared_app_table(0x02, /* appAsLib */ true);
  shared_app_table.addMapping(0x7f, 0x02);
  Res_value value2;
  value2.dataType = Res_value::TYPE_REFERENCE;
  value2.data = 0x7f010000;
  ASSERT_EQ(shared_app_table.lookupResourceValue(&value2), NO_ERROR);
  EXPECT_EQ(value2.data, 0x02010000);
};

TEST(DynamicRefTableTest, LookupSharedLibSelfAttributes) {
  // Shared library
  DynamicRefTable shared_table(0x03, /* appAsLib */ false);
  shared_table.addMapping(0x00, 0x03);
  Res_value value;
  value.dataType = Res_value::TYPE_ATTRIBUTE;
  value.data = 0x00010000;
  ASSERT_EQ(shared_table.lookupResourceValue(&value), NO_ERROR);
  EXPECT_EQ(value.data, 0x03010000);

  // App loaded as a shared library
  DynamicRefTable shared_app_table(0x04, /* appAsLib */ true);
  shared_app_table.addMapping(0x7f, 0x04);
  Res_value value2;
  value2.dataType = Res_value::TYPE_ATTRIBUTE;
  value2.data = 0x7f010000;
  ASSERT_EQ(shared_app_table.lookupResourceValue(&value2), NO_ERROR);
  EXPECT_EQ(value2.data, 0x04010000);
};

TEST(DynamicRefTableTest, LookupDynamicReferences) {
  // Shared library
  DynamicRefTable shared_table(0x2, /* appAsLib */ false);
  shared_table.addMapping(0x00, 0x02);
  shared_table.addMapping(0x03, 0x05);
  Res_value value;
  value.dataType = Res_value::TYPE_DYNAMIC_REFERENCE;
  value.data = 0x03010000;
  ASSERT_EQ(shared_table.lookupResourceValue(&value), NO_ERROR);
  EXPECT_EQ(value.data, 0x05010000);

  // Regular application
  DynamicRefTable app_table(0x7f, /* appAsLib */ false);
  app_table.addMapping(0x03, 0x05);
  Res_value value3;
  value3.dataType = Res_value::TYPE_DYNAMIC_REFERENCE;
  value3.data = 0x03010000;
  ASSERT_EQ(app_table.lookupResourceValue(&value3), NO_ERROR);
  EXPECT_EQ(value3.data, 0x05010000);
};

TEST(DynamicRefTableTest, LookupDynamicAttributes) {
// App loaded as a shared library
  DynamicRefTable shared_app_table(0x2, /* appAsLib */ true);
  shared_app_table.addMapping(0x03, 0x05);
  shared_app_table.addMapping(0x7f, 0x2);
  Res_value value2;
  value2.dataType = Res_value::TYPE_DYNAMIC_ATTRIBUTE;
  value2.data = 0x03010000;
  ASSERT_EQ(shared_app_table.lookupResourceValue(&value2), NO_ERROR);
  EXPECT_EQ(value2.data, 0x05010000);
}

TEST(DynamicRefTableTest, DoNotLookupNonDynamicReferences) {
  // Regular application
  DynamicRefTable app_table(0x7f, /* appAsLib */ false);
  Res_value value;
  value.dataType = Res_value::TYPE_REFERENCE;
  value.data = 0x03010000;
  ASSERT_EQ(app_table.lookupResourceValue(&value), NO_ERROR);
  EXPECT_EQ(value.data, 0x03010000);
};

TEST(DynamicRefTableTest, DoNotLookupNonDynamicAttributes) {
  // App with custom package id
  DynamicRefTable custom_app_table(0x8f, /* appAsLib */ false);
  Res_value value2;
  value2.dataType = Res_value::TYPE_ATTRIBUTE;
  value2.data = 0x03010000;
  ASSERT_EQ(custom_app_table.lookupResourceValue(&value2), NO_ERROR);
  EXPECT_EQ(value2.data, 0x03010000);
};

} // namespace cdroid
