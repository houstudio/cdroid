/*********************************************************************************
 * Copyright (C) [2026] [houzh@msn.com]
 *
 * Minimal BMP decoder supporting multiple bpp (1/2/4/8/16/24/32), RLE4/RLE8, BITFIELDS,
 * and BI_JPEG/BI_PNG by delegating to existing JPEG/PNG decoders.
 *********************************************************************************/

#include <core/context.h>
#include <cairomm/surface.h>
#include <image-decoders/imagedecoder.h>
#include <drawable/drawable.h>
#include <porting/cdlog.h>
#include <cstring>
#include <cstdint>
#include <vector>
#include <memory>
#include <sstream>

namespace cdroid {

struct BMPDecoder::PRIVATE {
    // Header parsing results
    uint32_t fileSize;
    uint32_t pixelDataOffset;
    uint32_t dibHeaderSize;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    uint32_t colorsUsed;
    uint32_t redMask, greenMask, blueMask, alphaMask;
    std::vector<uint8_t> palette; // raw palette bytes (BGRx or BGRA depending)
    std::vector<uint8_t> pixelData; // raw pixel stream copied from file (for BI_JPEG/BI_PNG)
};

static inline uint16_t readLE16(const uint8_t* p) {
    return (uint16_t)p[0] | (uint16_t)p[1] << 8;
}
static inline uint32_t readLE32(const uint8_t* p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

BMPDecoder::BMPDecoder(std::istream& stream): ImageDecoder(stream) {
    mPrivate = new PRIVATE();
}

BMPDecoder::~BMPDecoder() {
    delete mPrivate;
}

bool BMPDecoder::isBMP(const uint8_t* contents, uint32_t header_size) {
    return header_size >= 2 && contents[0] == 'B' && contents[1] == 'M';
}

bool BMPDecoder::decodeSize() {
    // Read minimal header: BITMAPFILEHEADER (14) + BITMAPINFOHEADER min (40)
    uint8_t header[64];
    mStream.seekg(0, std::ios::beg);
    mStream.read(reinterpret_cast<char*>(header), sizeof(header));
    const std::streamsize got = mStream.gcount();
    if (got < 14) return false;
    if (header[0] != 'B' || header[1] != 'M') return false;

    mPrivate->fileSize = readLE32(header + 2);
    mPrivate->pixelDataOffset = readLE32(header + 10);

    if (got < 14 + 4) return false;
    mPrivate->dibHeaderSize = readLE32(header + 14);
    if (mPrivate->dibHeaderSize < 12 || mPrivate->dibHeaderSize > 124) return false;

    if (mPrivate->dibHeaderSize == 12) {
        // OS/2 BITMAPCOREHEADER
        if (got < 26) return false;
        mPrivate->width = (int32_t)readLE16(header + 18);
        mPrivate->height = (int32_t)readLE16(header + 20);
        mPrivate->planes = readLE16(header + 22);
        mPrivate->bitsPerPixel = readLE16(header + 24);
        mPrivate->compression = 0;
        mPrivate->colorsUsed = 0;
    } else {
        // Windows BITMAPINFOHEADER or larger
        if (got < 14 + 40) return false;
        mPrivate->width = (int32_t)readLE32(header + 18);
        mPrivate->height = (int32_t)readLE32(header + 22);
        mPrivate->planes = readLE16(header + 26);
        mPrivate->bitsPerPixel = readLE16(header + 28);
        mPrivate->compression = readLE32(header + 30);
        mPrivate->imageSize = readLE32(header + 34);
        mPrivate->colorsUsed = readLE32(header + 46);
        // For BITFIELDS and BITMAPV4/V5 we may have color masks
        if (mPrivate->dibHeaderSize >= 52) {
            mPrivate->redMask = readLE32(header + 54);
            mPrivate->greenMask = readLE32(header + 58);
            mPrivate->blueMask = readLE32(header + 62);
            mPrivate->alphaMask = (mPrivate->dibHeaderSize >= 108) ? readLE32(header + 106) : 0;
        } else {
            mPrivate->redMask = mPrivate->greenMask = mPrivate->blueMask = mPrivate->alphaMask = 0;
        }
    }

    // Read palette if present (for bpp <= 8)
    if (mPrivate->bitsPerPixel <= 8) {
        uint32_t paletteEntries = mPrivate->colorsUsed ? mPrivate->colorsUsed : (1u << mPrivate->bitsPerPixel);
        size_t paletteBytes = paletteEntries * 4; // usually B G R 0
        mPrivate->palette.resize(paletteBytes);
        mStream.seekg(14 + mPrivate->dibHeaderSize, std::ios::beg);
        mStream.read(reinterpret_cast<char*>(mPrivate->palette.data()), paletteBytes);
    }

    // Read pixel data block (copy to memory for easier handling and for BI_JPEG/BI_PNG)
    mStream.seekg(0, std::ios::end);
    std::streampos endpos = mStream.tellg();
    size_t fileSize = static_cast<size_t>(endpos);
    if (mPrivate->pixelDataOffset >= fileSize) return false;
    size_t pixelBytes = fileSize - mPrivate->pixelDataOffset;
    mPrivate->pixelData.resize(pixelBytes);
    mStream.seekg(mPrivate->pixelDataOffset, std::ios::beg);
    mStream.read(reinterpret_cast<char*>(mPrivate->pixelData.data()), pixelBytes);

    mImageWidth = mPrivate->width;
    mImageHeight = std::abs(mPrivate->height);
    mFrameCount = 1;
    return true;
}

// Simple helpers for palette reading and writing pixels
static inline uint32_t readPaletteColor(const uint8_t* paletteEntry) {
    // palette entry: B G R [A?]
    uint8_t b = paletteEntry[0];
    uint8_t g = paletteEntry[1];
    uint8_t r = paletteEntry[2];
    return (uint32_t(255) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
}

// RLE8 decode: expands into width*height indices
static bool decodeRLE8(const uint8_t* src, size_t srcSize, int width, int height, std::vector<uint8_t>& outIndices) {
    outIndices.assign(width * height, 0);
    int x = 0, y = 0;
    size_t i = 0;
    while (i + 1 <= srcSize && y < height) {
        uint8_t a = src[i++];
        if (i >= srcSize) break;
        uint8_t b = src[i++];
        if (a > 0) {
            // replicate b, a times
            for (int k = 0; k < a && x < width; ++k) {
                outIndices[y * width + x] = b;
                x++;
            }
        } else {
            if (b == 0) { // end of line
                x = 0; y++;
            } else if (b == 1) { // end of bitmap
                return true;
            } else if (b == 2) { // delta
                if (i + 1 >= srcSize) return false;
                uint8_t dx = src[i++];
                uint8_t dy = src[i++];
                x += dx; y += dy;
            } else {
                // b = count of raw data
                int count = b;
                if (i + count > srcSize) return false;
                for (int k = 0; k < count && x < width; ++k) {
                    outIndices[y * width + x] = src[i++];
                    x++;
                }
                // pad to even
                if (count & 1) i++;
            }
        }
    }
    return true;
}

// Helpers for mask decoding
static inline int trailingZeros(uint32_t mask) {
#if defined(__GNUG__)
    return mask ? __builtin_ctz(mask) : 0;
#else
    int s = 0; while (mask && !(mask & 1u)) { mask >>= 1; s++; } return s;
#endif
}
static inline int popCount(uint32_t mask) {
#if defined(__GNUG__)
    return __builtin_popcount(mask);
#else
    int c = 0; while (mask) { c += (mask & 1u); mask >>= 1; } return c;
#endif
}

static inline uint8_t expandTo8(uint32_t value, int bits) {
    if (bits <= 0) return 0;
    if (bits >= 8) return static_cast<uint8_t>(value >> (bits - 8));
    // scale value to 0..255: (value * 255 + half) / max
    uint32_t maxv = (1u << bits) - 1u;
    return static_cast<uint8_t>((value * 255u + (maxv / 2u)) / maxv);
}

// RLE4 decode: similar but nibbles
static bool decodeRLE4(const uint8_t* src, size_t srcSize, int width, int height, std::vector<uint8_t>& outIndices) {
    outIndices.assign(width * height, 0);
    int x = 0, y = 0;
    size_t i = 0;
    while (i + 1 <= srcSize && y < height) {
        uint8_t a = src[i++];
        if (i >= srcSize) break;
        uint8_t b = src[i++];
        if (a > 0) {
            // a repetitions of two-index byte b (high nibble then low nibble)
            for (int k = 0; k < a && x < width; ++k) {
                uint8_t idx = (k % 2 == 0) ? (b >> 4) : (b & 0x0F);
                outIndices[y * width + x] = idx;
                x++;
            }
        } else {
            if (b == 0) {
                x = 0; y++;
            } else if (b == 1) {
                return true;
            } else if (b == 2) {
                if (i + 1 >= srcSize) return false;
                uint8_t dx = src[i++];
                uint8_t dy = src[i++];
                x += dx; y += dy;
            } else {
                int count = b;
                int nibbles = count;
                int bytes = (nibbles + 1) / 2;
                if (i + bytes > srcSize) return false;
                int nibbleIndex = 0;
                for (int k = 0; k < nibbles && x < width; ++k) {
                    uint8_t byteVal = src[i + (nibbleIndex/2)];
                    uint8_t idx = (nibbleIndex % 2 == 0) ? (byteVal >> 4) : (byteVal & 0x0F);
                    outIndices[y * width + x] = idx;
                    x++; nibbleIndex++;
                }
                i += bytes;
                if ((bytes & 1) == 1) i++; // pad to even
            }
        }
    }
    return true;
}

Cairo::RefPtr<Cairo::ImageSurface> BMPDecoder::decode(float scale, void* targetProfile) {
    if (mImageWidth == -1 || mImageHeight == -1) {
        if (!decodeSize()) return nullptr;
    }
    const int width = mImageWidth;
    const int height = mImageHeight;

    // Handle BI_JPEG (4) and BI_PNG (5) by delegating to ImageDecoder
    if (mPrivate->compression == 4 || mPrivate->compression == 5) {
        const uint8_t* data = mPrivate->pixelData.data();
        size_t sz = mPrivate->pixelData.size();
        size_t start = SIZE_MAX;
        size_t end = sz;

        // Try find PNG signature first
        const uint8_t pngSig[8] = {0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A};
        for (size_t i = 0; i + 8 <= sz; ++i) {
            if (std::memcmp(data + i, pngSig, 8) == 0) { start = i; break; }
        }
        if (start == SIZE_MAX) {
            // Try find JPEG SOI marker
            for (size_t i = 0; i + 2 <= sz; ++i) {
                if (data[i] == 0xFF && data[i + 1] == 0xD8) { start = i; break; }
            }
            if (start != SIZE_MAX) {
                // try find EOI marker
                for (size_t i = start + 2; i + 2 <= sz; ++i) {
                    if (data[i] == 0xFF && data[i + 1] == 0xD9) { end = i + 2; break; }
                }
            }
        } else {
            // For PNG try to locate IEND chunk to truncate accurately
            for (size_t i = start + 8; i + 12 <= sz; ++i) {
                if (data[i] == 'I' && data[i+1] == 'E' && data[i+2] == 'N' && data[i+3] == 'D') {
                    // IEND chunk is 12 bytes (length(4)=0 + 'IEND' + CRC(4))
                    end = i + 8 + 4; // i points to 'I', include up to CRC
                    break;
                }
            }
            if (end > sz) end = sz;
        }

        if (start == SIZE_MAX) {
            // fallback: try the whole blob
            start = 0; end = sz;
        }

        std::string embedded(reinterpret_cast<const char*>(data + start), end - start);
        std::istringstream s(embedded);
        std::unique_ptr<ImageDecoder> dec = ImageDecoder::getDecoder(s);
        if (dec) {
            return dec->decode(scale, targetProfile);
        }

        // Last resort: try whole pixelData blob
        std::istringstream s2(std::string(reinterpret_cast<const char*>(data), sz));
        dec = ImageDecoder::getDecoder(s2);
        if (dec) {
            return dec->decode(scale, targetProfile);
        }

        LOGE("Failed to locate embedded JPEG/PNG in BMP pixel data");
        return nullptr;
    }

    // For paletted formats or uncompressed formats, create ARGB surface and fill pixels
    Cairo::RefPtr<Cairo::ImageSurface> image = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, width, height);
    uint8_t* dstPixelsBase = image->get_data();
    const int dstStride = image->get_stride();

    if (mPrivate->compression == 1) { // RLE8
        std::vector<uint8_t> indices;
        if (!decodeRLE8(mPrivate->pixelData.data(), mPrivate->pixelData.size(), width, height, indices)) return nullptr;
        // Map indices to palette
        for (int y = 0; y < height; ++y) {
            uint32_t* dstRow = reinterpret_cast<uint32_t*>(dstPixelsBase + y * dstStride);
            for (int x = 0; x < width; ++x) {
                uint8_t idx = indices[(height - 1 - y) * width + x]; // BMP bottom-up
                uint8_t* pal = mPrivate->palette.data() + idx * 4;
                dstRow[x] = readPaletteColor(pal);
            }
        }
        ImageDecoder::setTransparency(image, ImageDecoder::computeTransparency(image));
        return image;
    }

    if (mPrivate->compression == 2) { // RLE4
        std::vector<uint8_t> indices;
        if (!decodeRLE4(mPrivate->pixelData.data(), mPrivate->pixelData.size(), width, height, indices)) return nullptr;
        for (int y = 0; y < height; ++y) {
            uint32_t* dstRow = reinterpret_cast<uint32_t*>(dstPixelsBase + y * dstStride);
            for (int x = 0; x < width; ++x) {
                uint8_t idx = indices[(height - 1 - y) * width + x];
                uint8_t* pal = mPrivate->palette.data() + idx * 4;
                dstRow[x] = readPaletteColor(pal);
            }
        }
        ImageDecoder::setTransparency(image, ImageDecoder::computeTransparency(image));
        return image;
    }

    // Handle uncompressed bitfields/truecolor formats
    if (mPrivate->bitsPerPixel == 24) {
        // 24bpp BGR triplets with row padding to 4 bytes
        const uint8_t* src = mPrivate->pixelData.data();
        size_t srcRowBytes = ((width * 3 + 3) / 4) * 4;
        for (int y = 0; y < height; ++y) {
            const uint8_t* srcRow = src + (height - 1 - y) * srcRowBytes;
            uint32_t* dstRow = reinterpret_cast<uint32_t*>(dstPixelsBase + y * dstStride);
            for (int x = 0; x < width; ++x) {
                uint8_t b = srcRow[x*3 + 0];
                uint8_t g = srcRow[x*3 + 1];
                uint8_t r = srcRow[x*3 + 2];
                dstRow[x] = (uint32_t(255) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
            }
        }
        ImageDecoder::setTransparency(image, ImageDecoder::computeTransparency(image));
        return image;
    }

    if (mPrivate->bitsPerPixel == 16) {
        // 16bpp: support BITFIELDS (masks) or fall back to common 5-6-5 layout
        const uint8_t* src = mPrivate->pixelData.data();
        size_t srcRowBytes = ((width * 2 + 3) / 4) * 4; // rows aligned to 4 bytes
        uint32_t rmask = mPrivate->redMask;
        uint32_t gmask = mPrivate->greenMask;
        uint32_t bmask = mPrivate->blueMask;
        uint32_t amask = mPrivate->alphaMask;
        if (rmask == 0 && gmask == 0 && bmask == 0) {
            // Default to RGB565 (common), if you need RGB555 set masks explicitly via BITFIELDS
            rmask = 0xF800u; gmask = 0x07E0u; bmask = 0x001Fu; amask = 0;
        }
        const int rshift = trailingZeros(rmask);
        const int gshift = trailingZeros(gmask);
        const int bshift = trailingZeros(bmask);
        const int rbits = popCount(rmask >> rshift);
        const int gbits = popCount(gmask >> gshift);
        const int bbits = popCount(bmask >> bshift);

        for (int y = 0; y < height; ++y) {
            const uint8_t* srcRow = src + (height - 1 - y) * srcRowBytes;
            uint32_t* dstRow = reinterpret_cast<uint32_t*>(dstPixelsBase + y * dstStride);
            for (int x = 0; x < width; ++x) {
                uint16_t pix = (uint16_t)srcRow[x*2] | ((uint16_t)srcRow[x*2 + 1] << 8);
                uint32_t rv = (rmask ? ((pix & rmask) >> rshift) : 0);
                uint32_t gv = (gmask ? ((pix & gmask) >> gshift) : 0);
                uint32_t bv = (bmask ? ((pix & bmask) >> bshift) : 0);
                uint8_t r = expandTo8(rv, rbits);
                uint8_t g = expandTo8(gv, gbits);
                uint8_t b = expandTo8(bv, bbits);
                uint8_t a = 255;
                if (amask) {
                    int ashift = trailingZeros(amask);
                    int abits = popCount(amask >> ashift);
                    a = expandTo8((pix & amask) >> ashift, abits);
                }
                dstRow[x] = (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
            }
        }
        ImageDecoder::setTransparency(image, ImageDecoder::computeTransparency(image));
        return image;
    }

    if (mPrivate->bitsPerPixel == 32) {
        // Either BGRA or BGR with bitfields. We'll assume BGRA or use masks if present
        const uint8_t* src = mPrivate->pixelData.data();
        size_t srcRowBytes = width * 4;
        for (int y = 0; y < height; ++y) {
            const uint8_t* srcRow = src + (height - 1 - y) * srcRowBytes;
            uint32_t* dstRow = reinterpret_cast<uint32_t*>(dstPixelsBase + y * dstStride);
            for (int x = 0; x < width; ++x) {
                uint8_t b = srcRow[x*4 + 0];
                uint8_t g = srcRow[x*4 + 1];
                uint8_t r = srcRow[x*4 + 2];
                uint8_t a = srcRow[x*4 + 3];
                dstRow[x] = (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
            }
        }
        ImageDecoder::setTransparency(image, ImageDecoder::computeTransparency(image));
        return image;
    }

    if (mPrivate->bitsPerPixel <= 8) {
        // Uncompressed paletted
        const uint8_t* src = mPrivate->pixelData.data();
        size_t srcRowBytes = ((width * mPrivate->bitsPerPixel + 31) / 32) * 4;
        for (int y = 0; y < height; ++y) {
            const uint8_t* srcRow = src + (height - 1 - y) * srcRowBytes;
            uint32_t* dstRow = reinterpret_cast<uint32_t*>(dstPixelsBase + y * dstStride);
            int bitPos = 0;
            for (int x = 0; x < width; ++x) {
                int idx = 0;
                if (mPrivate->bitsPerPixel == 8) {
                    idx = srcRow[x];
                } else if (mPrivate->bitsPerPixel == 4) {
                    uint8_t byte = srcRow[x/2];
                    idx = (x % 2 == 0) ? (byte >> 4) : (byte & 0x0F);
                } else if (mPrivate->bitsPerPixel == 2) {
                    int shift = 6 - (x % 4) * 2;
                    idx = (srcRow[x/4] >> shift) & 0x03;
                } else if (mPrivate->bitsPerPixel == 1) {
                    int shift = 7 - (x % 8);
                    idx = (srcRow[x/8] >> shift) & 0x01;
                }
                uint8_t* pal = mPrivate->palette.data() + idx * 4;
                dstRow[x] = readPaletteColor(pal);
            }
        }
        ImageDecoder::setTransparency(image, ImageDecoder::computeTransparency(image));
        return image;
    }

    // Unsupported configuration
    LOGE("Unsupported BMP format: bpp=%d compression=%u", mPrivate->bitsPerPixel, mPrivate->compression);
    return nullptr;
}

} // namespace cdroid
