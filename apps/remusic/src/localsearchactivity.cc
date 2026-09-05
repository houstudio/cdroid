// Port of com.wm.remusic.activity.LocalSearchActivity — SearchView over the
// local library. SearchHistory lands with the polish pass; results filter
// live and play on tap.
#include <algorithm>

#include <cdroid.h>
#include <R.h>
#include <text/String.h>
#include <core/activityfactory.h>
#include <fragment/fragmentactivity.h>
#include <widget/adapter.h>
#include <widget/edittext.h>
#include <widget/framelayout.h>
#include <widget/linearlayout.h>
#include <widget/listview.h>
#include <widget/textview.h>

#include <content/sharedpreferences.h>

#include "musicplayer.h"
#include "musicprovider.h"
#include "themestore.h"

using namespace cdroid;
using namespace remusic;

namespace {

class LocalSearchActivity : public FragmentActivity {
public:
    LocalSearchActivity() : FragmentActivity(0, 0, -1, -1) {}

    void onCreate(Bundle* savedInstanceState) override {
        FragmentActivity::onCreate(savedInstanceState);
        auto* root = new LinearLayout(getContext());
        root->setOrientation(LinearLayout::VERTICAL);
        root->setBackgroundColor(0xFFF2F2F2);

        auto* header = new LinearLayout(getContext());
        header->setOrientation(LinearLayout::HORIZONTAL);
        header->setBackgroundColor(ThemeStore::get().accent());
        auto* back = new TextView(getContext());
        back->setText("< 搜索");
        back->setTextColor(0xFFFFFFFF);
        back->setTextSize(18);
        back->setPadding(20, 16, 20, 16);
        back->setClickable(true);
        back->setOnClickListener([this](View&) { close(); });
        header->addView(back, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT, 56));
        mInput = new EditText(getContext());
        mInput->setTextColor(0xFFFFFFFFu);
        mInput->setHint("搜索本地音乐");
        mInput->setPadding(16, 8, 16, 8);
        header->addView(mInput, new LinearLayout::LayoutParams(
                0, 56, 1.f));
        root->addView(header);

        mList = new ListView(getContext());
        root->addView(mList, new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
        addView(root);

        MusicProvider::get().scanIfNeeded();
        mAll = MusicProvider::get().queryMusic(MusicProvider::SORT_ORDER_A_Z);
        TextWatcher watcher;   // NoCopySpan: must not outlive as a copy
        watcher.afterTextChanged = [this](Editable&) { requery(); };
        mInput->addTextChangedListener(watcher);
        requery();
    }

private:
    void requery() {
        String* value = mInput->getText().toString();
        const std::string q = value ? value->str() : std::string();
        delete value;
        std::string needle = q;
        std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);

        mMatches.clear();
        std::vector<std::string> rows;
        for (auto& s : mAll) {
            std::string hay = s.musicName + " " + s.artist;
            std::transform(hay.begin(), hay.end(), hay.begin(), ::tolower);
            if (needle.empty() || hay.find(needle) != std::string::npos) {
                mMatches.push_back(s);
                rows.push_back(s.musicName + " - " + s.artist);
            }
        }
        // SearchHistory: an empty query lists the remembered terms instead.
        auto history = [this] {
            std::vector<std::string> out;
            const std::string blob = getContext()->getSharedPreferences("searchhistory", 0)
                                             ->getString("terms", "");
            size_t start = 0;
            while (start < blob.size()) {
                const size_t nl = blob.find('\n', start);
                out.push_back(blob.substr(start,
                        nl == std::string::npos ? std::string::npos : nl - start));
                if (nl == std::string::npos) break;
                start = nl + 1;
            }
            return out;
        }();
        if (needle.empty() && !history.empty()) {
            rows = history;
            auto* adapter = new ArrayAdapter<std::string>(
                    getContext(), R::layout::design_drawer_item, 0);
            adapter->addAll(rows);
            mList->setAdapter(adapter);
            mList->setOnItemClickListener([this, history](AdapterView&, View&, int position, long) {
                if (position < (int)history.size()) {
                    String* cur = mInput->getText().toString();
                    delete cur;
                    mInput->setText(history[position]);
                }
            });
            return;
        }
        if (rows.empty()) rows.push_back("无匹配结果");
        auto* adapter = new ArrayAdapter<std::string>(
                getContext(), R::layout::design_drawer_item, 0);
        adapter->addAll(rows);
        mList->setAdapter(adapter);
        mList->setOnItemClickListener([this, q](AdapterView&, View&, int position, long) {
            if (position >= (int)mMatches.size()) return;
            if (!q.empty()) rememberSearch(q);
            std::map<long, MusicInfo> infos;
            std::vector<long> list;
            for (auto& s : mMatches) { infos[s.songId] = s; list.push_back(s.songId); }
            MusicPlayer::playAll(infos, list, position, false);
        });
    }

    void rememberSearch(const std::string& term) {
        auto prefs = getContext()->getSharedPreferences("searchhistory", 0);
        const std::string blob = prefs->getString("terms", "");
        std::vector<std::string> terms;
        size_t start = 0;
        while (start < blob.size()) {
            const size_t nl = blob.find('\n', start);
            terms.push_back(blob.substr(start,
                    nl == std::string::npos ? std::string::npos : nl - start));
            if (nl == std::string::npos) break;
            start = nl + 1;
        }
        terms.erase(std::remove(terms.begin(), terms.end(), term), terms.end());
        terms.insert(terms.begin(), term);
        if (terms.size() > 20) terms.resize(20);
        std::string out;
        for (auto& t : terms) out += t + "\n";
        prefs->edit().putString("terms", out).apply();
    }

    EditText* mInput = nullptr;
    ListView* mList = nullptr;
    std::vector<MusicInfo> mAll;
    std::vector<MusicInfo> mMatches;
};
REGISTER_ACTIVITY(LocalSearchActivity);

} // namespace
