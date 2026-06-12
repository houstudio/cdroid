/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FontFeatureUtils.h"

#include "StringPiece.h"

namespace minikin {

std::vector<hb_feature_t> cleanAndAddDefaultFontFeatures(const MinikinPaint& paint) {
    std::vector<hb_feature_t> features;
    // Disable default-on non-required ligature features if letter-spacing
    // See http://dev.w3.org/csswg/css-text-3/#letter-spacing-property
    // "When the effective spacing between two characters is not zero (due to
    // either justification or a non-zero value of letter-spacing), user agents
    // should not apply optional ligatures."
    if (fabs(paint.letterSpacing) > 0.03) {
        static constexpr hb_feature_t no_liga = {HB_TAG('l', 'i', 'g', 'a'), 0, 0, ~0u};
        static constexpr hb_feature_t no_clig = {HB_TAG('c', 'l', 'i', 'g'), 0, 0, ~0u};
        features.push_back(no_liga);
        features.push_back(no_clig);
    }

    bool default_enable_chws = true;

    static constexpr hb_tag_t chws_tag = HB_TAG('c', 'h', 'w', 's');
    static constexpr hb_tag_t halt_tag = HB_TAG('h', 'a', 'l', 't');
    static constexpr hb_tag_t palt_tag = HB_TAG('p', 'a', 'l', 't');

    SplitIterator it(paint.fontFeatureSettings, ',');
    while (it.hasNext()) {
        StringPiece featureStr = it.next();
        static hb_feature_t feature;
        // We do not allow setting features on ranges. As such, reject any setting that has
        // non-universal range.
        if (hb_feature_from_string(featureStr.data(), featureStr.size(), &feature) &&
            feature.start == 0 && feature.end == (unsigned int)-1) {
            // OpenType requires disabling default `chws` feature if glyph-width features.
            // https://docs.microsoft.com/en-us/typography/opentype/spec/features_ae#tag-chws
            // Here, we follow Chrome's impl: not enabling default `chws` feature if `palt` or
            // `halt` is enabled.
            // https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/renderer/platform/fonts/shaping/font_features.cc;drc=77a9a09de0688ca449f5333a305ceaf3f36b6daf;l=215
            if (default_enable_chws &&
                (feature.tag == chws_tag ||
                 (feature.value && (feature.tag == halt_tag || feature.tag == palt_tag)))) {
                default_enable_chws = false;
            }

            features.push_back(feature);
        }
    }

    if (default_enable_chws) {
        static constexpr hb_feature_t chws = {chws_tag, 1, 0, ~0u};
        features.push_back(chws);
    }

    return features;
}

}  // namespace minikin
