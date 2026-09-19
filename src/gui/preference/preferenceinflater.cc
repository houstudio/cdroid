/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <preference/preferenceinflater.h>
#include <preference/preference.h>
#include <preference/preferencegroup.h>
#include <preference/preferencescreen.h>
#include <preference/preferencemanager.h>
#include <core/xmlpullparser.h>
#include <content/resources.h>
#include <stdexcept>

namespace cdroid {

namespace {
std::map<std::string, PreferenceInflater::PreferenceInflaterFunc>& inflaterRegistry() {
    static std::map<std::string, PreferenceInflater::PreferenceInflaterFunc> registry;
    return registry;
}
constexpr const char* INTENT_TAG_NAME = "intent";
constexpr const char* EXTRA_TAG_NAME = "extra";
} // namespace

PreferenceInflater::PreferenceInflater(Context& context, PreferenceManager& preferenceManager)
    : mContext(context), mPreferenceManager(preferenceManager) {
}

bool PreferenceInflater::registerPreferenceInflater(const std::string& name,
        PreferenceInflaterFunc fun) {
    inflaterRegistry()[name] = std::move(fun);
    return true;
}

Context& PreferenceInflater::getContext() const {
    return mContext;
}

Preference* PreferenceInflater::inflate(int resource, PreferenceGroup* root) {
    auto parser = getContext().getResources().getXml(resource);
    return inflate(*parser, root);
}

Preference* PreferenceInflater::inflate(XmlPullParser& parser, PreferenceGroup* root) {
    const AttributeSet& attrs = parser; // Xml.asAttributeSet(parser)

    // Look for the root node.
    int type;
    do {
        type = parser.next();
    } while (type != XmlPullParser::START_TAG && type != XmlPullParser::END_DOCUMENT);

    if (type != XmlPullParser::START_TAG) {
        throw std::runtime_error(parser.getPositionDescription() + ": No start tag found!");
    }

    // Temp is the root that was found in the xml
    Preference* xmlRoot = createItemFromTag(parser.getName(), attrs);

    Preference* result = onMergeRoots(root, *static_cast<PreferenceGroup*>(xmlRoot));

    // Inflate all children under temp
    rInflate(parser, result, attrs);

    return result;
}

PreferenceGroup* PreferenceInflater::onMergeRoots(PreferenceGroup* givenRoot,
        PreferenceGroup& xmlRoot) {
    // If we were given a Preferences, use it as the root (ignoring the root
    // Preferences from the XML file).
    if (givenRoot == nullptr) {
        xmlRoot.onAttachedToHierarchy(mPreferenceManager);
        return &xmlRoot;
    } else {
        delete &xmlRoot;
        return givenRoot;
    }
}

Preference* PreferenceInflater::createItem(const std::string& name, const AttributeSet& attrs) {
    auto& registry = inflaterRegistry();
    auto it = registry.find(name);
    if (it == registry.end()) {
        throw std::runtime_error(attrs.getPositionDescription()
                + ": Error inflating class (not found) " + name);
    }
    return it->second(mContext, attrs);
}

Preference* PreferenceInflater::createItemFromTag(const std::string& name,
        const AttributeSet& attrs) {
    // AOSP checks for a '.' in the tag to decide between the prefixed lookup
    // and the fully-qualified class name; CDROID's registry is keyed by the
    // bare class name only (fully-qualified names are not supported without
    // reflection).
    if (name.find('.') != std::string::npos) {
        throw std::runtime_error(attrs.getPositionDescription()
                + ": Fully-qualified preference class names are not supported: " + name);
    }
    return createItem(name, attrs);
}

void PreferenceInflater::rInflate(XmlPullParser& parser, Preference* parent,
        const AttributeSet& attrs) {
    const int depth = parser.getDepth();

    int type;
    while (((type = parser.next()) != XmlPullParser::END_TAG ||
            parser.getDepth() > depth) && type != XmlPullParser::END_DOCUMENT) {

        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        const std::string name = parser.getName();

        if (INTENT_TAG_NAME == name) {
            // CDROID has no Intent.parseIntent(Resources, XmlPullParser,
            // AttributeSet) — the <intent> tag is skipped with its subtree.
            skipCurrentTag(parser);
        } else if (EXTRA_TAG_NAME == name) {
            // AOSP: getResources().parseBundleExtra("extra", attrs,
            // parent.getExtras()) — CDROID's Resources does not expose
            // parseBundleExtra; extras set from XML are not supported.
            skipCurrentTag(parser);
        } else {
            Preference* item = createItemFromTag(name, attrs);
            static_cast<PreferenceGroup*>(parent)->addItemFromInflater(item);
            rInflate(parser, item, attrs);
        }
    }
}

void PreferenceInflater::skipCurrentTag(XmlPullParser& parser) {
    const int outerDepth = parser.getDepth();
    int type;
    do {
        type = parser.next();
    } while (type != XmlPullParser::END_DOCUMENT
            && (type != XmlPullParser::END_TAG || parser.getDepth() > outerDepth));
}

} // namespace cdroid
