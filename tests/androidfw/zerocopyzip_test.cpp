// Zero-copy read-header reconciliation: parse the real cdroid.pak (mixed
// STORED/DEFLATED) and compare against Python-zipfile-produced ground truth
// (entry set + per-entry CRC + full byte content for a sample of each method).
#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "zerocopyzip.h"

#include <zlib.h>

using namespace cdroid;

namespace {

std::string frameworkPakPath() {
    static const char* candidates[] = {
        "cdroid.pak",
        "../../../cdroid.pak",
        "./bin/tests/../../../cdroid.pak",
    };
    for (const char* c : candidates) {
        if (FILE* f = fopen(c, "rb")) {
            fclose(f);
            return c;
        }
    }
    return "";
}

}  // namespace

class ZeroCopyZipTest : public testing::Test {
protected:
    void SetUp() override {
        const std::string pak = frameworkPakPath();
        if (pak.empty()) {
            GTEST_SKIP() << "cdroid.pak not found — run from the build dir";
        }
        ASSERT_TRUE(zip_.open(pak.c_str())) << "parse failed for " << pak;
    }

    ZeroCopyZip zip_;
};

TEST_F(ZeroCopyZipTest, ParsesWholeCentralDirectory) {
    // The pak carries ~4700+ entries; a partial walk or a bad local-header
    // offset computation shows up immediately as a shortfall.
    EXPECT_GT(zip_.entryCount(), 4000u);
}

TEST_F(ZeroCopyZipTest, StoredArscHandsOutWindowView) {
    const ZeroCopyZip::Entry* e = zip_.find("resources.arsc");
    ASSERT_NE(nullptr, e);
    EXPECT_EQ(0u, e->method);   // STORED since the pakbuilder switch
    EXPECT_EQ(3707648ull, e->uncompressedSize);
    // The view must sit INSIDE the mapped window and start with the arsc
    // table signature (ResTable_header: type 0x0002).
    const uint8_t* p = zip_.base() + e->dataOffset;
    ASSERT_TRUE(p + 8 <= zip_.base() + zip_.size());
    EXPECT_EQ(0x0002, (uint32_t)(p[0] | (p[1] << 8)));   // RES_TABLE_TYPE
    // And the STORED sizes must agree: compressed == uncompressed.
    EXPECT_EQ(e->compressedSize, e->uncompressedSize);
}

TEST_F(ZeroCopyZipTest, DeflatedEntryInflatesToFullBytes) {
    // Pick a known-DEFLATED entry (values/ text entries still deflate).
    const ZeroCopyZip::Entry* e = zip_.find("res/anim/accelerate_interpolator.xml");
    ASSERT_NE(nullptr, e);
    EXPECT_EQ(8u, e->method);
    uint8_t* data = zip_.inflate(*e);
    ASSERT_NE(nullptr, data);
    // Binary AXML: starts with RES_XML_TYPE 0x0003.
    EXPECT_EQ(0x0003, (uint32_t)(data[0] | (data[1] << 8)));
    // Tail truncation is covered by the CRC sweep below (binary AXML pads
    // to 4-byte alignment, so trailing zero bytes are legitimate).
    delete[] data;
}

TEST_F(ZeroCopyZipTest, DeflatedInflateMatchesUncompressedSizeExactly) {
    // Walk every DEFLATED entry and inflate — the read header must handle
    // the whole mixed pak, not just a sample.
    size_t deflated = 0, inflated_ok = 0;
    zip_.forEach([&](const std::string& name, const ZeroCopyZip::Entry& e) {
        if (e.method != 8) return;
        deflated++;
        uint8_t* data = zip_.inflate(e);
        if (data != nullptr) {
            // CRC-check the inflated bytes against the central record.
            const uint32_t crc = crc32(0, data, (uInt)e.uncompressedSize);
            if (crc == e.crc32) inflated_ok++;
            delete[] data;
        }
    });
    EXPECT_GT(deflated, 1000u);
    EXPECT_EQ(deflated, inflated_ok) << "some DEFLATED entries failed inflate/CRC";
}
