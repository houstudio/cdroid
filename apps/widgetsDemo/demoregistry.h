#pragma once
#include <string>
#include <vector>
#include <functional>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>

namespace cdroid {

// One registered demo: a "/"-pathed label (ApiDemos' label convention, e.g.
// "Views/Buttons") plus a fresh-instance factory and a tab-order rank (the
// pre-refactor pages keep their original tab order at ranks 1..N; ported
// ApiDemos pages default after them). The category shell derives its
// navigation tree from the path strings — exactly how ApiDemos' list Activity
// derives its hierarchy from CATEGORY_SAMPLE_CODE labels.
struct DemoEntry {
    std::string path;
    std::function<Fragment*()> create;
    int rank = 1000;
};

// Static registry fed at static-init by REGISTER_DEMO_FRAGMENT. Adding a demo
// is one line in its own .cc (the declarative equivalent of ApiDemos' manifest
// intent-filter): no central list to edit, registration is naturally incremental.
class DemoRegistry {
public:
    static DemoRegistry& get();
    void add(std::string path, std::function<Fragment*()> create, int rank = 1000);

    // All entries in registration order (static-init order — do not rely on it
    // for display; the queries below sort).
    const std::vector<DemoEntry>& entries() const { return mEntries; }

    // Sorted, unique child category names directly under prefix ("" = root).
    // A category exists only because some deeper demo lives under it.
    std::vector<std::string> categories(const std::string& prefix) const;

    // Leaf demos whose parent path is exactly prefix, sorted by last segment.
    std::vector<const DemoEntry*> demos(const std::string& prefix) const;

    // ALL leaves under prefix, recursively, sorted by full path — the flat tab
    // set of a category screen (sub-categories' demos are pulled up as tabs).
    std::vector<const DemoEntry*> leavesUnder(const std::string& prefix) const;

private:
    std::vector<DemoEntry> mEntries;
};

} // namespace cdroid

// Declare + register a demo Fragment under its "/"-pathed label. The class is
// ALSO registered with FragmentFactory (REGISTER_FRAGMENT), because back-stack
// / state restore re-instantiates fragments by className through the factory —
// the class needs a default ctor, and per-instance identity must ride the
// arguments Bundle, never members. Rank orders the tab strip (defaults after
// the pre-refactor pages).
#define REGISTER_DEMO_FRAGMENT_IMPL(PathValue, ClassName, RankValue)                  \
    static const int _cdroid_frag_reg_##ClassName =                                    \
        (::cdroid::FragmentFactory::registerFragment(                                  \
             #ClassName, []() -> ::cdroid::Fragment* { return new ClassName(); }), 0); \
    static const int _cdroid_demo_reg_##ClassName =                                    \
        (::cdroid::DemoRegistry::get().add(                                            \
             PathValue, []() -> ::cdroid::Fragment* { return new ClassName(); },      \
             RankValue), 0)
#define REGISTER_DEMO_FRAGMENT(PathValue, ClassName) \
    REGISTER_DEMO_FRAGMENT_IMPL(PathValue, ClassName, 1000)
