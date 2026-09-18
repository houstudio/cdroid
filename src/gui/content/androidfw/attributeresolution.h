// Port of AOSP androidfw/AttributeResolution.h (frameworks/base/libs/androidfw/
// include/androidfw/AttributeResolution.h), namespace cdroid.
//
// The STYLE_* layout constants are the typed-array wire format shared with
// android.content.res.TypedArray — they land now. The three resolver function
// BODIES (ResolveAttrs / ApplyStyle / RetrieveAttributes) are one unit with
// AssetManager2+Theme (they resolve through Theme::GetAttribute /
// AssetManager2::ResolveBag — the A12 AttributeResolution.cpp is AM2's styled-
// attribute layer), so they port together with assetmanager2.{h,cc} in the
// final stage, not before: no stub bodies, no half-wired seam.
//
// Copyright (C) 2016 The Android Open Source Project
// Licensed under the Apache License, Version 2.0.
#ifndef __CDROID_ANDROIDFW_ATTRIBUTERESOLUTION_H__
#define __CDROID_ANDROIDFW_ATTRIBUTERESOLUTION_H__

#include <cstddef>
#include <cstdint>

#include <content/androidfw/expected.h>
#include <content/androidfw/resourcetypes.h>

namespace cdroid {

// Error seam (expected.h) lives one namespace deeper.
using androidfw::IOError;
using androidfw::NullOrIOError;

class AssetManager2;
class Theme;

// Offsets into the outValues array populated by the methods below. outValues is a uint32_t
// array, but each logical element takes up 7 uint32_t-sized physical elements.
// Keep these in sync with android.content.res.TypedArray java class
enum {
  STYLE_NUM_ENTRIES = 7,
  STYLE_TYPE = 0,
  STYLE_DATA = 1,
  STYLE_ASSET_COOKIE = 2,
  STYLE_RESOURCE_ID = 3,
  STYLE_CHANGING_CONFIGURATIONS = 4,
  STYLE_DENSITY = 5,
  STYLE_SOURCE_RESOURCE_ID = 6
};

// These are all variations of the same method. They each perform the exact same operation,
// but on various data sources. I *think* they are re-written to avoid an extra branch
// in the inner loop, but after one branch miss (some pointer != null), the branch predictor should
// predict the rest of the iterations' branch correctly.
// TODO(adamlesinski): Run performance tests against these methods and a new, single method
// that uses all the sources and branches to the right ones within the inner loop.

// `out_values` must NOT be nullptr.
// `out_indices` may be nullptr.
base::expected<base::monostate, IOError> ResolveAttrs(Theme* theme, uint32_t def_style_attr,
                                                      uint32_t def_style_resid, const uint32_t* src_values,
                                                      size_t src_values_length, const uint32_t* attrs,
                                                      size_t attrs_length, uint32_t* out_values,
                                                      uint32_t* out_indices);

// `out_values` must NOT be nullptr.
// `out_indices` is NOT optional and must NOT be nullptr.
base::expected<base::monostate, IOError> ApplyStyle(Theme* theme, const ResXMLParser* xml_parser,
                                                    uint32_t def_style_attr,
                                                    uint32_t def_style_resid,
                                                    const uint32_t* attrs, size_t attrs_length,
                                                    uint32_t* out_values, uint32_t* out_indices);

// `out_values` must NOT be nullptr.
// `out_indices` may be nullptr.
base::expected<base::monostate, IOError> RetrieveAttributes(AssetManager2* assetmanager,
                                                            const ResXMLParser* xml_parser,
                                                            const uint32_t* attrs,
                                                            size_t attrs_length,
                                                            uint32_t* out_values,
                                                            uint32_t* out_indices);

}  // namespace cdroid

#endif  // __CDROID_ANDROIDFW_ATTRIBUTERESOLUTION_H__
