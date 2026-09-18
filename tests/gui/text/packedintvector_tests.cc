// Ported from AOSP coretests PackedIntVectorTest — the row-gap/value-gap
// int matrix behind DynamicLayout. Single exhaustive testBasic sweep.
// Original: frameworks/base/core/tests/coretests/src/android/text/PackedIntVectorTest.java (Apache 2.0)
//
// CDROID adaptation:
//  - android.util.PackedIntVector -> cdroid::PackedIntVector (text/packedintvector.h).
//  - insertAt(at, null) inserts a row of zeros; C++ takes const std::vector<int>&,
//    so pass an explicitly zeroed vector (same observable behavior).
#include <gtest/gtest.h>
#include <text/packedintvector.h>
#include <vector>

using namespace cdroid;

TEST(PackedIntVectorTest, testBasic) {
    for (int width = 0; width < 10; width++) {
        PackedIntVector p(width);
        std::vector<int> ins(width, 0);
        const std::vector<int> zeros(width, 0); // insertAt(at, null) equivalent

        for (int height = width * 2; height < width * 4; height++) {
            EXPECT_EQ(p.width(), width);

            // Test adding rows.

            for (int i = 0; i < height; i++) {
                int at;

                if (i % 2 == 0) {
                    at = i;
                } else {
                    at = p.size() - i;
                }

                for (int j = 0; j < width; j++) {
                    ins[j] = i + j;
                }

                if (i == height / 2) {
                    p.insertAt(at, zeros);
                } else {
                    p.insertAt(at, ins);
                }

                EXPECT_EQ(p.size(), i + 1);

                for (int j = 0; j < width; j++) {
                    if (i == height / 2) {
                        EXPECT_EQ(0, p.getValue(at, j));
                    } else {
                        EXPECT_EQ(p.getValue(at, j), i + j);
                    }
                }
            }

            // Test setting values.

            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    p.setValue(i, j, i * j);

                    EXPECT_EQ(p.getValue(i, j), i * j);
                }
            }

            // Test offsetting values.

            for (int j = 0; j < width; j++) {
                p.adjustValuesBelow(j * 2, j, j + 27);
            }

            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    int expect = i * j;

                    if (i >= j * 2) {
                        expect += j + 27;
                    }

                    EXPECT_EQ(p.getValue(i, j), expect);
                }
            }

            for (int j = 0; j < width; j++) {
                p.adjustValuesBelow(j, j, j * j + 14);
            }

            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    int expect = i * j;

                    if (i >= j * 2) {
                        expect += j + 27;
                    }
                    if (i >= j) {
                        expect += j * j + 14;
                    }

                    EXPECT_EQ(p.getValue(i, j), expect);
                }
            }

            // Test undoing offsets.

            for (int j = 0; j < width; j++) {
                p.adjustValuesBelow(j * 2, j, -(j + 27));
                p.adjustValuesBelow(j, j, -(j * j + 14));
            }

            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    EXPECT_EQ(p.getValue(i, j), i * j);
                }
            }

            // Test deleting rows.

            while (p.size() > 0) {
                int osize = p.size();
                int del = osize / 3;

                if (del == 0) {
                    del = 1;
                }

                int at = (osize - del) / 2;
                p.deleteAt(at, del);

                EXPECT_EQ(p.size(), osize - del);

                for (int i = 0; i < at; i++) {
                    for (int j = 0; j < width; j++) {
                        EXPECT_EQ(p.getValue(i, j), i * j);
                    }
                }

                for (int i = at; i < p.size(); i++) {
                    for (int j = 0; j < width; j++) {
                        EXPECT_EQ(p.getValue(i, j), (i + height - p.size()) * j);
                    }
                }
            }

            EXPECT_EQ(0, p.size());
        }
    }
}
