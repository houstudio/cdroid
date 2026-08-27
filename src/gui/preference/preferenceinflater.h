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
#ifndef __CDROID_PREFERENCE_INFLATER_H__
#define __CDROID_PREFERENCE_INFLATER_H__

#include <string>
#include <map>
#include <functional>
#include <core/context.h>
#include <core/attributeset.h>

namespace cdroid {

class Preference;
class PreferenceGroup;
class PreferenceManager;
class XmlPullParser;

/**
 * Port of androidx.preference.PreferenceInflater — this inflater is used to
 * inflate preference hierarchies from XML files.
 *
 * AOSP constructs preference classes reflectively (Class.forName + the
 * (Context, AttributeSet) constructor). CDROID has no reflection: the same
 * static-factory registry pattern as LayoutInflater (DECLARE_WIDGET) is used,
 * so every Preference subclass registers its tag name via DECLARE_PREFERENCE
 * and XML files use the bare class name (CheckBoxPreference, SwitchPreference…).
 */
class PreferenceInflater {
public:
    using PreferenceInflaterFunc = std::function<Preference*(Context&, const AttributeSet&)>;

    explicit PreferenceInflater(Context& context, PreferenceManager& preferenceManager);

    /**
     * Registers a factory for the given tag name (the C++ counterpart of
     * AOSP's CONSTRUCTOR_MAP reflection cache).
     */
    static bool registerPreferenceInflater(const std::string& name, PreferenceInflaterFunc fun);

    Context& getContext() const;

    /**
     * Inflate a new item hierarchy from the specified xml resource.
     */
    Preference* inflate(int resource, PreferenceGroup* root);
    /**
     * Inflate a new hierarchy from the specified XML node.
     */
    Preference* inflate(XmlPullParser& parser, PreferenceGroup* root);

    std::string getPreferenceClassName() const { return "PreferenceInflater"; }

private:
    PreferenceGroup* onMergeRoots(PreferenceGroup* givenRoot, PreferenceGroup& xmlRoot);
    Preference* createItem(const std::string& name, const AttributeSet& attrs);
    Preference* createItemFromTag(const std::string& name, const AttributeSet& attrs);
    void rInflate(XmlPullParser& parser, Preference* parent, const AttributeSet& attrs);
    static void skipCurrentTag(XmlPullParser& parser);

    Context& mContext;
    PreferenceManager& mPreferenceManager;
};

/**
 * Static registration helper: place `DECLARE_PREFERENCE(T)` in the .cc of a
 * Preference subclass to make tag "T" inflatable from preference XML.
 */
template <typename T>
struct PreferenceInflaterRegister {
    PreferenceInflaterRegister(const std::string& name) {
        PreferenceInflater::registerPreferenceInflater(name,
            [](Context& ctx, const AttributeSet& attrs) -> Preference* {
                return new T(ctx, attrs);
            });
    }
};

#define DECLARE_PREFERENCE(T) \
    static PreferenceInflaterRegister<T> preference_inflater_##T(#T);

} // namespace cdroid

#endif // __CDROID_PREFERENCE_INFLATER_H__
