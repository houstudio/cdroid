/*********************************************************************************
 * NavArgument tests — port of androidx.navigation navigation-common NavArgumentTest
 * (the subset CDROID implements).
 *
 * Ported: putDefaultValue (string/int/bool defaults land in the Bundle typed),
 * verify (present arg passes; missing arg passes only when nullable or has
 * default — the androidx null-array case maps to "missing" here since CDROID's
 * BaseBundle has no putNull).
 *
 * Skipped: setUnknownDefaultValuePresent (Builder has no
 * setUnknownDefaultValuePresent API).
 *********************************************************************************/
#include <gtest/gtest.h>
#include <navigation/navargument.h>
#include <navigation/navtype.h>

using namespace cdroid;

TEST(NavArgument, PutDefaultValue) {
    Bundle bundle;
    NavArgument* argument = NavArgument::Builder()
        .setDefaultValue(std::string("abc"))
        .setType(NavTypeKind::STRING)
        .build();
    argument->putDefaultValue("name", bundle);
    EXPECT_EQ(bundle.getString("name"), std::string("abc"));
    delete argument;
}

TEST(NavArgument, PutDefaultValueTyped) {
    Bundle bundle;
    NavArgument* intArg = NavArgument::Builder()
        .setDefaultValue(123).setType(NavTypeKind::INT).build();
    NavArgument* boolArg = NavArgument::Builder()
        .setDefaultValue(true).setType(NavTypeKind::BOOL).build();
    intArg->putDefaultValue("count", bundle);
    boolArg->putDefaultValue("flag", bundle);
    EXPECT_EQ(bundle.getInt("count"), 123);
    EXPECT_TRUE(bundle.getBoolean("flag"));
    delete intArg;
    delete boolArg;
}

TEST(NavArgument, PutDefaultValueNoDefaultIsNoop) {
    Bundle bundle;
    NavArgument* argument = NavArgument::Builder().setType(NavTypeKind::STRING).build();
    argument->putDefaultValue("name", bundle);
    EXPECT_FALSE(bundle.containsKey("name"));
    delete argument;
}

TEST(NavArgument, Verify) {
    Bundle bundle;
    bundle.putString("stringArg", "abc");
    bundle.putInt("intArg", 123);

    NavArgument* stringArgument = NavArgument::Builder().setType(NavTypeKind::STRING).build();
    NavArgument* intArgument = NavArgument::Builder().setType(NavTypeKind::INT).build();
    NavArgument* nullableArgument = NavArgument::Builder()
        .setType(NavTypeKind::STRING).setIsNullable(true).build();
    NavArgument* defaultArgument = NavArgument::Builder()
        .setType(NavTypeKind::STRING).setDefaultValue(std::string("d")).build();

    EXPECT_TRUE (stringArgument->verify("stringArg", bundle));
    EXPECT_TRUE (intArgument->verify("intArg", bundle));
    EXPECT_FALSE(stringArgument->verify("missingArg", bundle)); // required, no default
    EXPECT_TRUE (nullableArgument->verify("missingArg", bundle)); // nullable -> passes
    EXPECT_TRUE (defaultArgument->verify("missingArg", bundle));  // has default -> passes

    delete stringArgument;
    delete intArgument;
    delete nullableArgument;
    delete defaultArgument;
}

TEST(NavArgument, BuilderInfersTypeFromDefaultValue) {
    // CDROID extension aligned with androidx inference: a defaultValue alone
    // determines the NavType when setType is not called.
    NavArgument* argument = NavArgument::Builder().setDefaultValue(42).build();
    EXPECT_EQ(argument->getType(), NavTypeKind::INT);
    EXPECT_TRUE(argument->isDefaultValuePresent());
    delete argument;
}
